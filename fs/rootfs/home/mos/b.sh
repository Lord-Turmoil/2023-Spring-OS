# Demonstrate ; and &

echo "Test B: Demonstration of ; and &"

/home/mos/timer 3000 aaa; /home/mos/timer 2000 bbb; /home/mos/timer 1000 ccc
/home/mos/timer 3000 aaa& /home/mos/timer 2000 bbb& /home/mos/timer 1000 ccc

exit
