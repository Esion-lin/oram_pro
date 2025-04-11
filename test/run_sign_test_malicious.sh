#!/bin/bash
output_file=test_sign_malicious_output.log
# 定义网络接口（根据实际情况修改）
INTERFACE="lo"
# 定义三种测试场景的参数
SCENARIOS=(
    "1ms:10gbit:LAN"    # RTT 1ms, 带宽 10Gbps
    "100ms:1000mbit:MAN" # RTT 100ms, 带宽 1000Mbps
    "200ms:100mbit:WAN"  # RTT 200ms, 带宽 100Mbps
)

# 捕获 Ctrl+C 信号并终止所有 test_sign_malicious 进程
trap 'echo "捕获到 Ctrl+C，终止所有 test_sign_malicious 进程..."; pkill -f test_sign_malicious; exit 1' SIGINT

cd ../build
make -j24
rm -rf ../test/malicious/
mkdir ../test/malicious/

# 遍历所有场景
for scenario in "${SCENARIOS[@]}"; do
    # 提取延迟和带宽参数
    DELAY=$(echo "$scenario" | cut -d':' -f1)
    BANDWIDTH=$(echo "$scenario" | cut -d':' -f2)
    NAME=$(echo "$scenario" | cut -d':' -f3)
    echo -e "\n===== 设置网络参数: RTT ${DELAY}, 带宽 ${BANDWIDTH} ====="

    # 清除现有流量控制规则
    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
    sudo tc qdisc add dev $INTERFACE root netem rate $BANDWIDTH delay $DELAY

    # 显示当前规则
    echo "当前流量控制规则:"
    sudo tc qdisc show dev $INTERFACE

    # 运行测试脚本
    echo -e "\n执行测试脚本..."
    # 从 2^4 到 2^18 的数据维度循环
    for ((dim=4; dim<=18; dim=dim+2))
    do
        # 计算当前数据维度的实际值 (10^dim)
        data_size=$((2**dim))
        
        echo "Running tasks with data size: $data_size" | tee -a "${NAME}_$output_file"
        
        # 运行三个子任务并将输出附加到文件
        ./test_sign_malicious player0 $data_size >> "../test/malicious/${NAME}_malicious_output_player0.log" 2>&1 &
        ./test_sign_malicious player1 $data_size >> "../test/malicious/${NAME}_malicious_output_player1.log" 2>&1 &
        ./test_sign_malicious player2 $data_size >> "../test/malicious/${NAME}_malicious_output_player2.log" 2>&1 &
        wait
        echo "Completed tasks for data size: $data_size" | tee -a "${NAME}_$output_file"
        echo "-------------------------------------------" | tee -a "${NAME}_$output_file"
    done

    # 清理规则（可选）
    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
done

echo "所有测试场景执行完毕！"