#include "Voice.h"
#include <string>
#include <vector>

/* System headers. */
#include <stdio.h>
#include <assert.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* SphinxBase headers. */
#include <sphinxbase/err.h>
#include <sphinxbase/strfuncs.h>
#include <sphinxbase/filename.h>
#include <sphinxbase/pio.h>
#include <sphinxbase/jsgf.h>
#include <sphinxbase/hash_table.h>

/* Local headers. */
#include "cmdln_macro.h"
#include "pocketsphinx.h"
#include "pocketsphinx_internal.h"
#include "ps_lattice_internal.h"
#include "phone_loop_search.h"
#include "kws_search.h"
#include "fsg_search_internal.h"
#include "ngram_search.h"
#include "ngram_search_fwdtree.h"
#include "ngram_search_fwdflat.h"
#include "allphone_search.h"

#pragma comment(lib, "pocketsphinx.lib")
#pragma comment(lib, "sphinxbase.lib")

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE *rawfd;

static ad_rec_t *ad;
static int16 adbuf[2048];
static uint8 utt_started, in_speech;
static int32 k;
static char const *hyp;
char last_voice_command[128] = "NA";
unsigned last_count = 0;
DWORD last_tick = 0;
DWORD tick_elapse = 0;
DWORD dwThreadId;

extern bool voice_left;
extern bool voice_right;
extern bool voice_up;
extern bool voice_down;

using namespace std;

static const arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    /* Argument file. */
    {"-argfile",
     ARG_STRING,
     NULL,
     "Argument file giving extra arguments."},
    {"-adcdev",
     ARG_STRING,
     NULL,
     "Name of audio device to use for input."},
    {"-infile",
     ARG_STRING,
     NULL,
     "Audio file to transcribe."},
    {"-inmic",
     ARG_BOOLEAN,
     "no",
     "Transcribe audio from microphone."},
    {"-time",
     ARG_BOOLEAN,
     "no",
     "Print word times in file transcription."},
    CMDLN_EMPTY_OPTION
};

int split(const string& str, vector<string>& ret_, string sep = " ")
{
    if (str.empty())
    {
        return 0;
    }

    string tmp;
    string::size_type pos_begin = str.find_first_not_of(sep);
    string::size_type comma_pos = 0;

    while (pos_begin != string::npos)
    {
        comma_pos = str.find(sep, pos_begin);
        if (comma_pos != string::npos)
        {
            tmp = str.substr(pos_begin, comma_pos - pos_begin);
            pos_begin = comma_pos + sep.length();
        }
        else
        {
            tmp = str.substr(pos_begin);
            pos_begin = comma_pos;
        }

        if (!tmp.empty())
        {
            ret_.push_back(tmp);
            tmp.clear();
        }
    }
    return 0;
}

static int
file_exists(const char *path)
{
    FILE *tmp;

    tmp = fopen(path, "rb");
    if (tmp) fclose(tmp);
    return (tmp != NULL);
}

static void ps_add_file(ps_decoder_t *ps, const char *arg,
            const char *hmmdir, const char *file)
{
    char *tmp = string_join(hmmdir, "/", file, NULL);

    if (cmd_ln_str_r(ps->config, arg) == NULL && file_exists(tmp))
        cmd_ln_set_str_r(ps->config, arg, tmp);
    ckd_free(tmp);
}

/* Feature and front-end parameters that may be in feat.params */
static const arg_t feat_defn[] = {
    waveform_to_cepstral_command_line_macro(),
    cepstral_to_feature_command_line_macro(),
    CMDLN_EMPTY_OPTION
};

static void
ps_expand_model_config2(ps_decoder_t *ps)
{
    char const *hmmdir, *featparams;

    /* Disable memory mapping on Blackfin (FIXME: should be uClinux in general). */
#ifdef __ADSPBLACKFIN__
    E_INFO("Will not use mmap() on uClinux/Blackfin.");
    cmd_ln_set_boolean_r(ps->config, "-mmap", FALSE);
#endif

    /* Get acoustic model filenames and add them to the command-line */
    if ((hmmdir = cmd_ln_str_r(ps->config, "-hmm")) != NULL) {
        ps_add_file(ps, "-mdef", hmmdir, "mdef");
        ps_add_file(ps, "-mean", hmmdir, "means");
        ps_add_file(ps, "-var", hmmdir, "variances");
        ps_add_file(ps, "-tmat", hmmdir, "transition_matrices");
        ps_add_file(ps, "-mixw", hmmdir, "mixture_weights");
        ps_add_file(ps, "-sendump", hmmdir, "sendump");
        ps_add_file(ps, "-fdict", hmmdir, "noisedict");
        ps_add_file(ps, "-lda", hmmdir, "feature_transform");
        ps_add_file(ps, "-featparams", hmmdir, "feat.params");
        ps_add_file(ps, "-senmgau", hmmdir, "senmgau");
    }

    /* Look for feat.params in acoustic model dir. */
    if ((featparams = cmd_ln_str_r(ps->config, "-featparams"))) {
        if (NULL !=
            cmd_ln_parse_file_r(ps->config, feat_defn, featparams, FALSE))
            E_INFO("Parsed model-specific feature parameters from %s\n",
                    featparams);
    }

    /* Print here because acmod_init might load feat.params file */
    if (err_get_logfp() != NULL) {
	//cmd_ln_print_values_r(ps->config, err_get_logfp(), ps_args());
    }
}

static void
ps_free_searches2(ps_decoder_t *ps)
{
    if (ps->searches) {
        hash_iter_t *search_it;
        for (search_it = hash_table_iter(ps->searches); search_it;
             search_it = hash_table_iter_next(search_it)) {
            ps_search_free((ps_search_t*)hash_entry_val(search_it->ent));
        }
        hash_table_free(ps->searches);
    }

    ps->searches = NULL;
    ps->search = NULL;
}


static int
set_search_internal(ps_decoder_t *ps, ps_search_t *search)
{
    ps_search_t *old_search;
    
    if (!search)
	return -1;

    search->pls = ps->phone_loop;
    old_search = (ps_search_t *) hash_table_replace(ps->searches, ps_search_name(search), search);
    if (old_search != search)
        ps_search_free(old_search);

    return 0;
}

ps_decoder_t * ps_init2(cmd_ln_t *config)
{
    ps_decoder_t *ps;
    
    if (!config) {
	E_ERROR("No configuration specified");
	return NULL;
    }

    ps = (ps_decoder_t*)ckd_calloc(1, sizeof(*ps));
    ps->refcount = 1;
    if (ps_reinit(ps, config) < 0) {
        ps_free(ps);
        return NULL;
    }
    return ps;
}

#define MODE 4
#define RESTART_AFTER_TIME_OUT
#define TIME_OUT 4000

void init_voice_ctrl()
{
  char const *cfg;
  const int argc = 15;
  char *argv[20];
  argv[0] = "xxx";
  argv[1] = "-inmic";
  argv[2] = "yes";

#if (MODE == 1)
  argv[3] = "-hmm";
  argv[4] = "new\\hmm";
  argv[5] = "-dict";
  argv[6] = "new\\my.dic";
  argv[7] = "-jsgf";
  argv[8] = "new\\my.jsgf";
#elif (MODE == 2)  // letter A,B,C
  argv[3] = "-hmm";
  argv[4] = "letter15\\hmm";
  argv[5] = "-dict";
  argv[6] = "letter15\\letter.dic";
  argv[7] = "-jsgf";
  argv[8] = "letter15\\letter.jsgf";
#elif (MODE == 3)  // chinese
  argv[3] = "-hmm";
  argv[4] = "chmodel30\\hmm";
  argv[5] = "-dict";
  argv[6] = "chmodel30\\m30.dic";
  argv[7] = "-jsgf";
  argv[8] = "chmodel30\\m30.jsgf";
#elif (MODE == 4)  // new chinese
  argv[3] = "-hmm";
  argv[4] = "sphinx_liu_yang\\hmm";
  argv[5] = "-dict";
  argv[6] = "sphinx_liu_yang\\s.dic";
  argv[7] = "-jsgf";
  argv[8] = "sphinx_liu_yang\\s.jsgf";
#else
  argv[3] = "-hmm";
  argv[4] = "hmm";
  argv[5] = "-dict";
  argv[6] = "voice\\game.dic";
  argv[7] = "-kws";
  argv[8] = "voice\\kws";
#endif

  argv[9] = "-ds";
  argv[10] = "3";
  argv[11] = "-topn";
  argv[12] = "2";
  argv[13] = "-maxhmmpf";
  argv[14] = "3000";
  argv[15] = "-maxwpf";
  argv[16] = "10";

  config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, TRUE);

  /* Handle argument file as -argfile. */
  if (config && (cfg = cmd_ln_str_r(config, "-argfile")) != NULL) 
  {
    config = cmd_ln_parse_file_r(config, cont_args_def, cfg, FALSE);
  }

  if (config == NULL)
  {
    E_INFO("Specify '-infile <file.wav>' to recognize from file or '-inmic yes' to recognize from microphone.\n");
    cmd_ln_free_r(config);
    return;
  }

  ps_default_search_args(config);
  ps = ps_init2(config);
  if (ps == NULL) 
  {
    cmd_ln_free_r(config);
    return;
  }

  E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);

  if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),
                        (int) cmd_ln_float32_r(config, "-samprate"))) == NULL)
  {
    E_FATAL("Failed to open audio device\n");
  }
  if (ad_start_rec(ad) < 0)
  {
    E_FATAL("Failed to start recording\n");
  }
  if (ps_start_utt(ps) < 0)
  {
    E_FATAL("Failed to start utterance\n");
  }
  utt_started = FALSE;
  ps_set_rawdata_size(ps, 512);
}

static void
sleep_msec(int32 ms)
{
#if (defined(_WIN32) && !defined(GNUWINCE)) || defined(_WIN32_WCE)
    Sleep(ms);
#else
    /* ------------------- Unix ------------------ */
    struct timeval tmo;

    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;

    select(0, NULL, NULL, NULL, &tmo);
#endif
}

DWORD tick;

DWORD recognize_from_microphone2(LPVOID lpdwThreadParam)
{
  last_count = 0;
  tick = GetTickCount();
    for (;;) {
        if ((k = ad_read(ad, adbuf, 2048)) < 0)
            E_FATAL("Failed to read audio\n");
        ps_process_raw(ps, adbuf, k, 0, 0);
        in_speech = ps_get_in_speech(ps);
        if (in_speech && !utt_started) {
            utt_started = TRUE;
        }

        if (!in_speech && utt_started) 
        {
          /* speech -> silence transition, time to start new utterance  */
          ps_end_utt(ps);
          hyp = ps_get_hyp(ps, NULL);
          last_count++;
          memset(last_voice_command, 0, 128);

          if (hyp == NULL)
          {
            strcat(last_voice_command, "NULL");
          }

          if (hyp != NULL && !strcmp(hyp, ""))
          {
            strcat(last_voice_command, "NA");
          }

          if (hyp != NULL && strcmp(hyp, "") )
          {
            char buf[16];
              
            strcat(last_voice_command, hyp);
            vector<string> strs;
            split(hyp, strs);
            string last = strs.at(strs.size() - 1);
            char * voice_str = (char*)last.c_str();
            memset(buf, 0, 16);
            for (int i = 0; i < strlen(voice_str); i++)
            {
              buf[i] = toupper(voice_str[i]);
            }
            voice_str = buf;
              
            /*
            if (strstr(voice_str, "UP"))
            {
              voice_up = true;
              voice_down = false;
              voice_right = false;
              voice_left = false;
            }

            if (strstr(voice_str, "DOWN"))
            {
              voice_up = false;
              voice_down = true;
              voice_right = false;
              voice_left = false;
            }
            */

            if (strstr(voice_str, "LEFT") ||
              strstr(voice_str, "A") )
            {
              voice_up = false;
              voice_down = false;
              voice_right = false;
              voice_left = true;

            }

            if (strstr(voice_str, "RIGHT") ||
              strstr(voice_str, "B") )
            {
              voice_up = false;
              voice_down = false;
              voice_right = true;
              voice_left = false;
            }

            if (strstr(voice_str, "STOP") ||
              strstr(voice_str, "C") )
            {
              voice_up = false;
              voice_down = false;
              voice_right = false;
              voice_left = false;
            }
              
          }

          if (ps_start_utt(ps) >= 0)
          {
            last_tick = GetTickCount();
          }
          utt_started = FALSE;
        }

        sleep_msec(3);

#ifdef RESTART_AFTER_TIME_OUT
        if (utt_started && last_tick != 0)
        {
          tick_elapse = GetTickCount() - last_tick;

          if (tick_elapse > TIME_OUT) // 
          {
            ps_end_utt(ps);

            ps_start_utt(ps);
            last_tick = GetTickCount();
            utt_started = FALSE;
          }
        }
#endif
    }
    ad_close(ad);
}

void start_void_ctrl()
{
  if (CreateThread(NULL, //Choose default security
      0, //Default stack size
      (LPTHREAD_START_ROUTINE)&recognize_from_microphone2,//Routine to execute
      (LPVOID) 0, //Thread parameter
      0, //Immediately run the thread
      &dwThreadId //Thread Id
      ) == NULL)
  {
    return;
  }
  else
  {
    //MessageBoxA(NULL, "OK", NULL, 0);
  }


    //ad_close(ad);
}