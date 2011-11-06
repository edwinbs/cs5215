for ((i=1; i<=21; i++))
do
  printf -v infile "comp%02d.ctt" $i
  for ((j=1; j<=20; j++))
  do
    printf -v outfile "sol%02d_with_seed_%02d.out" $i $j
    validator/validator testcases/$infile solutions/$outfile | grep "Total Cost = [0-9]*"
  done
done