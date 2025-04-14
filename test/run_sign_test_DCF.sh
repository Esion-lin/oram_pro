#!/bin/bash
test_name=DCF

output_file=test_sign_output.log
INTERFACE="lo"
SCENARIOS=(
    "1ms:10gbit:LAN"    # RTT 1ms,  10Gbps
    "100ms:1000mbit:MAN" # RTT 100ms,  1000Mbps
    "200ms:100mbit:WAN"  # RTT 200ms,  100Mbps
)

cd ../build
make -j40
rm -rf ../test/${test_name}/
mkdir ../test/${test_name}/
for scenario in "${SCENARIOS[@]}"; do
    DELAY=$(echo "$scenario" | cut -d':' -f1)
    BANDWIDTH=$(echo "$scenario" | cut -d':' -f2)
    SCENARIO=$(echo "$scenario" | cut -d':' -f3)
    echo -e "\n===== RTT ${DELAY}, ${BANDWIDTH} ====="

    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
    sudo tc qdisc add dev $INTERFACE root netem rate $BANDWIDTH delay $DELAY


    sudo tc qdisc show dev $INTERFACE


    for ((dim=4; dim<=18; dim+=2))
    do
        data_size=$((2**dim))
        
        echo "Running tasks with data size: $data_size" | tee -a "${SCENARIO}_$output_file"
        
        ./test_fss player1 $data_size >> "../test/${test_name}/${SCENARIO}_output_player1.log" 2>&1 &
        ./test_fss player2 $data_size >> "../test/${test_name}/${SCENARIO}_output_player2.log" 2>&1 &
        ./test_fss player0 $data_size >> "../test/${test_name}/${SCENARIO}_output_player0.log" 2>&1 &

        wait
        echo "Completed tasks for data size: $data_size" | tee -a "${SCENARIO}_$output_file"
        echo "-------------------------------------------" | tee -a "${SCENARIO}_$output_file"
    done

    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
done

