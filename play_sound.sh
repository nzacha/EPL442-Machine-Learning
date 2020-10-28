#amixer -c 1 -- sset Master playback 40%
aplay --quiet src/Media/tada.wav
#amixer -c 1 -- sset Master playback 100%
