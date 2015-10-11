@echo off
pocketsphinx_continuous -inmic yes -logfn log -ds 2 -topn 2 -hmm sphinx -jsgf sphinx\s.jsgf -dict sphinx\s.dic