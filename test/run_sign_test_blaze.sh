#!/bin/bash
output_file=test_sign_output.log
test_name=blaze
INTERFACE="lo"
SCENARIOS=(
    "1ms:10gbit:LAN"    # RTT 1ms,  10Gbps
    "100ms:1000mbit:MAN" # RTT 100ms,  1000Mbps
    "200ms:100mbit:WAN"  # RTT 200ms,  100Mbps
)

cd ../build
make -j24
rm -rf ../test/${test_name}/
mkdir ../test/${test_name}/

trap ' pkill -f test_blaze; exit 1' SIGINT

for scenario in "${SCENARIOS[@]}"; do
    DELAY=$(echo "$scenario" | cut -d':' -f1)
    BANDWIDTH=$(echo "$scenario" | cut -d':' -f2)
    NAME=$(echo "$scenario" | cut -d':' -f3)

    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
    sudo tc qdisc add dev $INTERFACE root netem rate $BANDWIDTH delay $DELAY


    sudo tc qdisc show dev $INTERFACE


    for ((dim=4; dim<=18; dim+=2))
    do
        data_size=$((2**dim))
        
        echo "Running tasks with data size: $data_size" | tee -a "${NAME}_$output_file"
        
        ./test_blaze player0 $data_size >> "../test/${test_name}/${NAME}_output_player0.log" 2>&1 &
        ./test_blaze player1 $data_size >> "../test/${test_name}/${NAME}_output_player1.log" 2>&1 &
        ./test_blaze player2 $data_size >> "../test/${test_name}/${NAME}_output_player2.log" 2>&1 &

        wait
        echo "Completed tasks for data size: $data_size" | tee -a "${NAME}_$output_file"
        echo "-------------------------------------------" | tee -a "${NAME}_$output_file"
    done

    sudo tc qdisc del dev $INTERFACE root 2>/dev/null || true
done

echo "test over"