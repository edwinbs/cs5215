for qfile in $1*
do
    echo $qfile
	./sudokusat $qfile -s 1 -i 50
done