#ssmtp zicolas3@gmail.com < msg.txt
#mail -s "Execution Done" zicolas3@gmail.com < mail.txt 
echo "sending mail"
mail -s "Execution Done" zicolas3@gmail.com <<< "Hello, execution has finished :)" 
