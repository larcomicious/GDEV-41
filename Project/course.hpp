
#pragma once

#include "track_builder.hpp"

struct Course {
    Track track;
    Vector2 startPosition;
    float finishX;
};

int subdivisions = 20;

inline Course createBeginnerCourse() {
    TrackBuilder builder({100.0f, 500.0f});

    // 1. Starting platform
    builder.addStraightTo({450.0f, 500.0f});

    // 2. Gentle rolling hills
    builder.addBezierTo(
        {600.0f, 500.0f},
        {650.0f, 400.0f},
        {800.0f, 420.0f},
        subdivisions
    );

    builder.addSmoothBezierTo(
        {1000.0f, 350.0f},
        {1200.0f, 430.0f},
        subdivisions
    );

    // 3. Steeper uphill section
    builder.addSmoothBezierTo(
        {1450.0f, 550.0f},
        {1650.0f, 450.0f},
        subdivisions
    );

    // 4. Uneven terrain
    builder.addStraightTo({1800.0f, 450.0f});

    builder.addBezierTo(
        {1900.0f, 450.0f},
        {1900.0f, 330.0f},
        {2050.0f, 350.0f},
        5
    );

    builder.addSmoothBezierTo(
        {2250.0f, 300.0f},
        {2400.0f, 430.0f},
        subdivisions
    );

    // 5. Final downhill stretch
    builder.addSmoothBezierTo(
        {2600.0f, 550.0f},
        {2800.0f, 500.0f},
        subdivisions
    );

    // 6. Finish platform
    builder.addStraightTo({3200.0f, 500.0f});

    return {
        builder.getTrack(),
        {150.0f, 500.0f},
        3100.0f
    };
}

inline Course createLevelOneCourse() {

    const int curve_subdivisions = 10;

    TrackBuilder builder({0.0f, 500.0f});

    builder.addStraightTo({400.0f, 500.0f});

    builder.addBezierTo(
        {605.0f, 500.5f},
        {529.0f, 537.0f},
        {690.5f, 546.0f},
        curve_subdivisions
    );

    builder.addBezierTo(
        {843.0f, 555.5f},  
        {885.0f, 512.3f},   
        {1021.5f, 489.0f},   
        curve_subdivisions
    );

    builder.addBezierTo(
        {1158.0f, 465.8f},
        {1261.6f, 411.4f},
        {1401.5f, 390.0f},
        curve_subdivisions
    );

    builder.addBezierTo(
        {1557.6f, 366.0f},  
        {1649.0f, 366.5f},   
        {1808.0f, 366.5f},   
        curve_subdivisions
    );

    builder.addBezierTo(
        {1975.0f, 366.5f},
        {2126.4f, 437.0f}, 
        {2328.0f, 442.5f}, 
        curve_subdivisions
    );

    builder.addBezierTo(
        {2580.0f, 449.5f}, 
        {2749.0f, 553.0f},   
        {2790.5f, 606.5f},    
        curve_subdivisions
    );

    builder.addBezierTo(
        {2832.0f, 660.0f}, 
        {2932.0f, 858.5f},   
        {3032.0f, 877.5f},    
        curve_subdivisions
    );

    builder.addStraightTo({3954.0f, 877.5f});

    builder.addBezierTo(
        {4084.7f, 883.3f},    
        {4368.3f, 929.5f},  
        {4457.5f, 1049.5f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {4569.0f, 1204.0f},  
        {4982.5f, 1234.5f},  
        {5211.5f, 1228.5f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {5440.5f, 1222.5f},  
        {5855.0f, 1122.5f},  
        {5935.0f, 1000.0f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {6015.0f, 877.5f},  
        {6534.0f, 795.5f},  
        {6800.0f, 789.5f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {7066.0f, 783.5f},  
        {7318.0f, 1000.0f},  
        {7600.0f, 1000.5f},  
        curve_subdivisions
    );

    builder.addStraightTo({8485.0f, 1000.0f});

    return {
        builder.getTrack(),
        {100.0f, 500.0f},
        8485.0f         
    };
}

inline Course createLevelTwoCourse() {
    const int curve_subdivisions = 10;

    TrackBuilder builder({0.0f, 858.0f});

    builder.addStraightTo({314.0f, 858.0f});

    builder.addBezierTo(
        {381.5f, 869.3f},  
        {531.0f, 904.7f},  
        {589.0f, 955.5f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {661.5f, 1019.0f},  
        {922.0f, 1089.0f},  
        {1031.5f, 1068.5f},  
        curve_subdivisions
    );
    
    builder.skipTo({1251.0f, 1180.5f});
    
    builder.addBezierTo(
        {1347.0f, 1221.5f},  
        {1514.0f, 1442.5f},  
        {1595.0f, 1450.5f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {1676.0f, 1458.5f},  
        {2003.0f, 1450.5f},  
        {2063.0f, 1450.5f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {2123.0f, 1450.5f},  
        {2153.5f, 1451.0f},  
        {2207.0f, 1395.5f},  
        curve_subdivisions
    );
    
    builder.skipTo({2354.0f, 1332.0f});

    builder.addBezierTo(
        {2471.0f, 1332.0f},  
        {2584.0f, 1363.5f},  
        {2612.5f, 1420.0f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {2650.1f, 1494.5f},  
        {2735.9f, 1525.6f},  
        {2773.0f, 1542.0f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {2836.5f, 1570.0f},  
        {3014.0f, 1578.0f},  
        {3043.0f, 1521.0f},  
        curve_subdivisions
    );

    builder.skipTo({3211.5f, 1450.5f});

    builder.addStraightTo({3849.0f, 1450.5f});

    builder.addBezierTo(
        {3917.3f, 1466.8f},  
        {4108.6f, 1478.5f},  
        {4327.0f, 1394.5f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {4600.0f, 1289.5f},  
        {4812.5f, 1331.0f},  
        {4953.0f, 1420.0f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {5112.5f, 1521.0f},  
        {5195.7f, 1686.8f},  
        {5419.5f, 1764.5f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {5612.5f, 1831.5f},  
        {5986.0f, 1819.0f},  
        {6126.0f, 1675.0f},  
        curve_subdivisions
    );

    builder.skipTo({6388.5f, 1536.5f});

    builder.addBezierTo(
        {6442.3f, 1497.6f},  
        {6578.0f, 1420.0f},  
        {6690.0f, 1420.0f},  
        curve_subdivisions
    );

    builder.addStraightTo({7285.0f, 1420.0f});

    builder.addBezierTo(
        {7366.5f, 1427.6f},  
        {7544.4f, 1456.1f},  
        {7604.0f, 1508.5f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {7678.5f, 1574.0f},  
        {7544.4f, 1456.1f},  
        {8103.0f, 1536.5f},  
        curve_subdivisions
    );

    return {
        builder.getTrack(),
        {100.0f, 858.0f},
        8103.0f         
    };
};

inline Course createLevelThreeCourse() {
    const int curve_subdivisions = 10;

    TrackBuilder builder({0.0f, 500.0f});

    builder.addStraightTo({466.0f, 500.0f});

    builder.addBezierTo(
        {578.5f, 539.7f},  
        {821.6f, 647.2f},  
        {894.0f, 743.0f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {1112.9f, 1029.6f},  
        {1211.4f, 1370.0f},  
        {1572.0f, 1370.0f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {1732.0f, 1370.0f},  
        {1802.5f, 1358.0f},  
        {1904.5f, 1283.5f},  
        curve_subdivisions
    );

    // circle
    builder.skipTo({2447.5f, 1124.1f});

    builder.addBezierTo(
        {2689.8f, 1102.2f},  
        {2868.5f, 888.0f},  
        {2846.5f, 645.7f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {2824.6f, 403.4f},  
        {2610.4f, 224.8f},  
        {2368.1f, 246.7f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {2125.8f, 268.7f},  
        {1947.2f, 482.9f},  
        {1969.1f, 725.2f},  
        curve_subdivisions
    );
    // endcircle

    builder.skipTo({1998.0f, 1578.5f});

    builder.addBezierTo(
        {2011.5f, 1735.5f},  
        {2163.5f, 2035.5f},  
        {2497.5f, 2038.0f},  
        curve_subdivisions
    );

    builder.addStraightTo({3594.5f, 2038.0f});

    builder.addBezierTo(
        {3727.3f, 2032.7f},  
        {4029.3f, 2005.3f},  
        {4174.5f, 1938.5f},  
        curve_subdivisions
    );

    builder.skipTo({4582.0f, 1781.5f});

    builder.addBezierTo(
        {4650.7f, 1757.0f},  
        {4830.2f, 1722.7f},  
        {4999.0f, 1781.5f},  
        curve_subdivisions
    );

    builder.addBezierTo(
        {5210.0f, 1855.0f},  
        {5336.0f, 1824.5f},  
        {5517.5f, 1749.0f},  
        curve_subdivisions
    );


    // circle
    builder.skipTo({6044.5f, 1617.1f});

    builder.addBezierTo(
        {6286.8f, 1595.2f},  
        {6465.4f, 1381.0f},  
        {6443.5f, 1138.7f},  
        curve_subdivisions
    );
    
    builder.addBezierTo(
        {6421.6f, 896.4f},  
        {6207.4f, 717.8f},  
        {5965.1f, 739.7f},  
        curve_subdivisions
    );
    
    builder.addBezierTo(
        {5722.8f, 761.7f},  
        {5544.2f, 975.9f},  
        {5566.1f, 1218.2f},  
        curve_subdivisions
    );
    // endcircle

    builder.skipTo({5623.0f, 1924.5f});

    builder.addBezierTo(
        {5686.8f, 2034.5f},  
        {6039.9f, 2259.4f},  
        {6301.5f, 2223.0f},  
        curve_subdivisions
    );

    builder.addStraightTo(
        {6855.0f, 2126.5f}
    );

    builder.skipTo({7285.5f, 2086.0f});

    builder.addStraightTo(
        {8388.5f, 2086.0f}
    );

    return {
        builder.getTrack(),
        {100.0f, 500.0f},
        8388.5f         
    };
}