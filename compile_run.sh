echo "compiling program.."
g++ -g -std=c++11 src/BackPropagation/bpRoutine.cpp -o executable.out
echo "executing program.."
echo
./executable.out
rm executable.out
./scripts/mail/send_mail.sh
./scripts/sound/play_sound.sh
