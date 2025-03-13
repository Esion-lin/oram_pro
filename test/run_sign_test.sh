#!/bin/bash
cd ../build
make -j40
output_file=./test_sign_output.log
# 从 2^4 到 2^18 的数据维度循环
for ((dim=2; dim<=6; dim++))
do
    # 计算当前数据维度的实际值 (2^dim)
    data_size=$((10**dim))
    
    echo "Running tasks with data size: $data_size" | tee -a "$output_file"
    
    # 运行三个子任务并将输出附加到文件
    ./test_sign player0 $data_size >> "output_player0.log" 2>&1 &
    ./test_sign player1 $data_size >> "output_player1.log" 2>&1 &
    ./test_sign player2 $data_size >> "output_player2.log" 2>&1 &
    wait
    echo "Completed tasks for data size: $data_size" | tee -a "$output_file"
    echo "-------------------------------------------" | tee -a "$output_file"
done

echo "All tasks completed. Output written to $output_file"
