Compare* cmp = new Compare(st, p2pchnl);
    //cmp->msb_off(4, 34, true);
    //cmp->twopc_ic_off(4, 1521571397, 2493325539, (uint64_t)1<<32, 32);
    /*4295912975, 8589946904, 1613827447, 6710252978*/
    /*1,        0,          0,          1*/
    uint64_t buffer[4];
    if(st == "aid"){
        Mul * mmul = new Mul(st, p2pchnl);
        mmul->offline();
        delete mmul;
    } 
    if(st == "player0"){
        uint32_t brr[5] = {1943807975,600768653,1433695274,72837182};
        Mul * mmul = new Mul(brr[0], brr[1], &brr[2], &brr[3], st, p2pchnl);
        mmul->offline();
        mmul->round1();
        mmul->round2();
        mmul->round3();
        mmul->roundend();
        delete mmul;
        // cmp->msb_1<uint32_t>(brr, 4,res,buffer, 34);
        // cmp->msb_diag_2<uint32_t>(brr, 4,res,buffer, 34, 0, true);
    }else if(st == "player1"){
        
        uint32_t brr[5] = {914631543,1433695274,87876123,2420488357};
        Mul * mmul = new Mul(brr[0], brr[1], &brr[2], &brr[3], st, p2pchnl);
        mmul->offline();
        mmul->round1();
        mmul->round2();
        mmul->round3();
        mmul->roundend();
        delete mmul;
        // cmp->msb_1<uint32_t>(brr, 4,res,buffer, 34);
        // cmp->msb_diag_2<uint32_t>(brr, 4,res,buffer, 34, 0, true);
    }
    else if(st == "player2"){
        uint32_t brr[5] = {1375911125,2420488357,10323231,4134994620};
        Mul * mmul = new Mul(brr[0], brr[1], &brr[2], &brr[3], st, p2pchnl);
        mmul->offline();
        mmul->round1();
        mmul->round2();
        mmul->round3();
        mmul->roundend();
        delete mmul;
        // cmp->msb_1<uint32_t>( brr, 4,res,buffer, 34);
        // cmp->msb_diag_2<uint32_t>(brr, 4,res,buffer, 34, 0, true);
    }
    else if(st == "player3"){
        uint32_t brr[5] = {60616654,4134994620,81932819,81932819};
        Mul * mmul = new Mul(brr[0], brr[1], &brr[2], &brr[3], st, p2pchnl);
        mmul->offline();
        mmul->round1();
        mmul->round2();
        mmul->round3();
        mmul->roundend();
        delete mmul;
        // cmp->msb_1<uint32_t>(brr, 4,res,buffer, 34);
        // cmp->msb_diag_2<uint32_t>(brr, 4,res,buffer, 34, 0, true);
    }
    twopc_reveal<uint32_t>(res, res2, 4, st, p2pchnl);
    for(int i = 0; i < 4; i++){
        std::cout<<"+ res "<<res2[i]<<std::endl;
    }
    delete cmp;
    std::cout<<"-----------------------test done-------------------\n";

    std::cout<<"-----------------------test overflow-------------------\n";
    cmp = new Compare(st, p2pchnl);
    Compare* cmp2 = new Compare(st, p2pchnl);
    cmp->scmp_off(2);
    cmp2->mul_off(2);
    Timer::record("overflow2");
    /*  520918858 1
        12312   12312
    */
    if(st == "player0"){
        uint32_t arr[2] = {824693088,1438719804};
        uint32_t brr[2] = {3071257272,4090056186};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->mul_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->mul_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);},true);
        cmp->overflow_end(arr, brr, 2,res);
    }else if(st == "player1"){
        uint32_t arr[2] = {2182846789,1047901216};
        uint32_t brr[2] = {3575917287,2964804424};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->mul_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->mul_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);},true);
        cmp->overflow_end(arr, brr, 2,res);
    }else if(st == "player2"){
        uint32_t arr[2] = {983653189,628188141};
        uint32_t brr[2] = {2977866476,210757332};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->mul_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->mul_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);},true);
        cmp->overflow_end(arr, brr, 2,res);
    }else if (st == "player3"){
        uint32_t arr[2] = {824693088,1180158136};
        uint32_t brr[2] = {3259873165,1324328962};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->mul_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->mul_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);},true);
        cmp->overflow_end(arr, brr, 2,res);
    }
    Timer::stop("overflow2");
    fourpc_reveal<uint32_t>(res, 2, {"player0"}, st, p2pchnl);
    for(int i = 0; i < 2; i++){
        std::cout<<"+ split res "<<res[i]<<std::endl;
    }
    fourpc_reveal<uint32_t>(res2, 2, {"player0"}, st, p2pchnl);
    for(int i = 0; i < 2; i++){
        std::cout<<"- split res "<<res2[i]<<std::endl;
    }
    delete cmp2;
    delete cmp;

    cmp = new Compare(st, p2pchnl);
    cmp->IC_OFF(4, 0, (uint64_t)1<<32, true);
    Timer::record("ic");
    if(st == "player0"){
        uint64_t arr[4] = {0,(uint64_t)41241, (uint64_t)1<<30, (uint64_t)1<<32};
        uint32_t brr[2] = {((uint32_t)1<<22)+22,123};
        cmp->IC<uint64_t>(arr,4, res, true);
    }else{
        uint64_t arr[4] = {0,(uint64_t)1, (uint64_t)1<<30, (uint64_t)1<<31};
        uint32_t brr[2] = {0,123};
        cmp->IC<uint64_t>(arr,4, res, true);
    }
    Timer::stop("ic");
    diag_reveal<uint32_t>(res, res2, 4,  st, p2pchnl);
    for(int i = 0; i < 4; i++){
        std::cout<<"== res "<<res2[i]<<std::endl;
    }
    delete cmp;