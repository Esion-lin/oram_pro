#!/bin/bash
test_name=DCF

output_file=test_sign_output.log
# 定义网络接口（根据实际情况修改）
INTERFACE="lo"
# 定义三种测试场景的参数
SCENARIOS=(
    "1ms:10gbit:LAN"    # RTT 1ms, 带宽 10Gbps
    "100ms:1000mbit:MAN" # RTT 100ms, 带宽 1000Mbps
    "200ms:100mbit:WAN"  # RTT 200ms, 带宽 100Mbps
)

cd ../build
make -j40
rm -rf ../test/${test_name}/
mkdir ../test/${test_name}/
# 遍历所有场景
for scenario in "${SCENARIOS[@]}"; do
    # 提取延迟和带宽参数
    DELAY=$(echo "$scenario" | cut -d':' -f1)
    BANDWIDTH=$(echo "$scenario" | cut -d':' -f2)
    SCENARIO=$(echo "$scenario" | cut -d':' -f3)
    echo -e "\n===== 设置网络参数: RTT ${DELAY}, 带宽 ${BANDWIDTH} ====="

    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
    sudo tc qdisc add dev $INTERFACE root netem rate $BANDWIDTH delay $DELAY


    # 显示当前规则
    echo "当前流量控制规则:"
    sudo tc qdisc show dev $INTERFACE


    # 运行测试脚本
    echo -e "\n执行测试脚本..."
    # 从 2^4 到 2^18 的数据维度循环
    for ((dim=4; dim<=18; dim+=2))
    do
        # 计算当前数据维度的实际值 (2^dim)
        data_size=$((2**dim))
        
        echo "Running tasks with data size: $data_size" | tee -a "${SCENARIO}_$output_file"
        
        # 运行三个子任务并将输出附加到文件
        ./test_fss player1 $data_size >> "../test/${test_name}/${SCENARIO}_output_player1.log" 2>&1 &
        ./test_fss player2 $data_size >> "../test/${test_name}/${SCENARIO}_output_player2.log" 2>&1 &
        ./test_fss player0 $data_size >> "../test/${test_name}/${SCENARIO}_output_player0.log" 2>&1 &

        wait
        echo "Completed tasks for data size: $data_size" | tee -a "${SCENARIO}_$output_file"
        echo "-------------------------------------------" | tee -a "${SCENARIO}_$output_file"
    done

    # 清理规则（可选）
    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
done

echo "所有测试场景执行完毕！"