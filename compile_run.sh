echo "compiling program.."
g++ -g -std=c++11 src/BackPropagation/bpRoutine.cpp -o executable.out
echo "executing program.."
echo
./executable.out
rm executable.out
#./play_sound.sh
./send_mail.sh
