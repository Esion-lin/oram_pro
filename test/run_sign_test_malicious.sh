#!/bin/bash
output_file=test_sign_malicious_output.log
INTERFACE="lo"
SCENARIOS=(
    "1ms:10gbit:LAN"    # RTT 1ms, 10Gbps
    "100ms:1000mbit:MAN" # RTT 100ms, 1000Mbps
    "200ms:100mbit:WAN"  # RTT 200ms, 100Mbps
)

trap 'pkill -f test_sign_malicious; exit 1' SIGINT

cd ../build
make -j24
rm -rf ../test/malicious/
mkdir ../test/malicious/

for scenario in "${SCENARIOS[@]}"; do
    DELAY=$(echo "$scenario" | cut -d':' -f1)
    BANDWIDTH=$(echo "$scenario" | cut -d':' -f2)
    NAME=$(echo "$scenario" | cut -d':' -f3)
    echo -e "\n===== RTT ${DELAY}, ${BANDWIDTH} ====="

    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
    sudo tc qdisc add dev $INTERFACE root netem rate $BANDWIDTH delay $DELAY

    sudo tc qdisc show dev $INTERFACE

    for ((dim=4; dim<=18; dim=dim+2))
    do
        data_size=$((2**dim))
        
        echo "Running tasks with data size: $data_size" | tee -a "${NAME}_$output_file"
        
        ./test_sign_malicious player0 $data_size >> "../test/malicious/${NAME}_malicious_output_player0.log" 2>&1 &
        ./test_sign_malicious player1 $data_size >> "../test/malicious/${NAME}_malicious_output_player1.log" 2>&1 &
        ./test_sign_malicious player2 $data_size >> "../test/malicious/${NAME}_malicious_output_player2.log" 2>&1 &
        wait
        echo "Completed tasks for data size: $data_size" | tee -a "${NAME}_$output_file"
        echo "-------------------------------------------" | tee -a "${NAME}_$output_file"
    done

    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
done

