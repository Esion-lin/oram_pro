#!/bin/bash
output_file=test_sign_output.log
INTERFACE="lo"
SCENARIOS=(
    "1ms:10gbit:LAN"    # RTT 1ms, 10Gbps
    "100ms:1000mbit:MAN" # RTT 100ms, 1000Mbps
    "200ms:100mbit:WAN"  # RTT 200ms, 100Mbps
)

cd ../build
make -j40
rm -rf ../test/semi-honest/
mkdir ../test/semi-honest/
for scenario in "${SCENARIOS[@]}"; do
    DELAY=$(echo "$scenario" | cut -d':' -f1)
    BANDWIDTH=$(echo "$scenario" | cut -d':' -f2)
    NAME=$(echo "$scenario" | cut -d':' -f3)
    echo -e "\n===== RTT ${DELAY}, ${BANDWIDTH} ====="

    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
    sudo tc qdisc add dev $INTERFACE root netem rate $BANDWIDTH delay $DELAY
    sudo tc qdisc show dev $INTERFACE

    for ((dim=4; dim<=18; dim+=2))
    do
        data_size=$((2**dim))
        
        echo "Running tasks with data size: $data_size" | tee -a "${NAME}_$output_file"
        
        ./test_sign player0 $data_size >> "../test/semi-honest/${NAME}_output_player0.log" 2>&1 &
        ./test_sign player1 $data_size >> "../test/semi-honest/${NAME}_output_player1.log" 2>&1 &
        ./test_sign player2 $data_size >> "../test/semi-honest/${NAME}_output_player2.log" 2>&1 &

        wait
        echo "Completed tasks for data size: $data_size" | tee -a "${NAME}_$output_file"
        echo "-------------------------------------------" | tee -a "${NAME}_$output_file"
    done

    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
done

