for ((i=1; i<=21; i++))
do
  printf -v filename "comp%02d" $i
  echo $filename.ctt
  for ((j=1; j<=5; j++))
  do
    java -cp build/classes main.main testcases/$filename.ctt -s $j -t 375 > solutions/$filename.sln
    validator/validator testcases/$filename.ctt solutions/$filename.sln | grep "Total Cost = [0-9]*"
  done
done