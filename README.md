Lightening FastSecure Comparison for 3PCPPML


```bash
mkdir build
cmake -S . -B build
cmake --build build --parallel
```

To Run the tests, use the scripts in ./test with bash
```bash
cd test
bash run_sign_test.sh
bash run_sign_test_blaze.sh
bash run_sign_test_DCF.sh
bash run_sign_test_malicious.sh
bash run_sign_test_bicoptor.sh
```


or you can run the test manually.(in 3 terminal)
```bash
        ./test_sign player0 $data_size
        ./test_sign player1 $data_size
        ./test_sign player2 $data_size
```