// ============================================
// RX Car Chassis - Compact ESP32 RC Car
// ============================================
// Parametric design for 3D printing
// Hot Wheels sized RC car chassis
// Holds: XIAO ESP32-C3, DRV8833, GS-1502 servo, 2x N20 motors, LiPo
//
// Author: Generated for ESP32 Joystick Project
// Version: 1.0
// License: MIT

// ============================================
// PARAMETERS - Adjust for your components
// ============================================

// Wall thickness
wall = 1.5;

// Overall chassis dimensions (Hot Wheels scale)
chassis_length = 80;    // Front to back
chassis_width = 32;     // Side to side
chassis_height = 18;    // Bottom to top (not including wheels)

// Wheel wells
wheel_dia = 20;         // Wheel diameter
wheel_width = 8;        // Wheel width
wheel_offset = 6;       // Distance from chassis edge

// N20 Motor dimensions
motor_dia = 12;
motor_length = 25;      // Including gearbox
motor_shaft_dia = 3;
motor_shaft_len = 10;

// GS-1502 Linear Servo
servo_width = 21;
servo_depth = 15;
servo_height = 12;

// Seeed XIAO ESP32-C3
xiao_width = 21;
xiao_depth = 17.5;
xiao_height = 4;

// DRV8833 Motor Driver
drv_width = 15;
drv_depth = 10;
drv_height = 3;

// LiPo Battery (501220 - 100mAh)
batt_width = 20;
batt_depth = 12;
batt_height = 5;

// Axle dimensions
axle_dia = 2;
axle_clearance = 0.3;

// ============================================
// CALCULATED POSITIONS
// ============================================

// Motor positions (rear)
motor_y = -chassis_length/2 + motor_length/2 + 5;
motor_x_offset = chassis_width/2 - motor_dia/2 - wall - 2;

// Servo position (front, for steering)
servo_y = chassis_length/2 - servo_depth/2 - 8;

// Electronics bay (center)
xiao_y = 0;
drv_y = motor_y + motor_length/2 + drv_depth/2 + 3;

// Battery position
batt_y = xiao_y + xiao_depth/2 + batt_depth/2 + 2;

// Wheel positions
front_axle_y = chassis_length/2 - 12;
rear_axle_y = -chassis_length/2 + 12;

// ============================================
// MODULES
// ============================================

// Rounded rectangle
module rounded_rect(w, d, h, r=2) {
    hull() {
        for (x = [-1, 1], y = [-1, 1]) {
            translate([x*(w/2-r), y*(d/2-r), 0])
                cylinder(h=h, r=r, $fn=24);
        }
    }
}

// Motor mount
module motor_mount() {
    difference() {
        // Mount block
        translate([0, 0, motor_dia/2])
            rotate([0, 90, 0])
                cylinder(d=motor_dia+4, h=motor_length+2, center=true, $fn=32);

        // Motor cavity
        translate([0, 0, motor_dia/2])
            rotate([0, 90, 0])
                cylinder(d=motor_dia+0.5, h=motor_length+5, center=true, $fn=32);

        // Shaft exit hole
        translate([motor_length/2+1, 0, motor_dia/2])
            rotate([0, 90, 0])
                cylinder(d=motor_shaft_dia+1, h=10, center=true, $fn=16);

        // Wire routing slot
        translate([0, 0, -1])
            cube([motor_length-5, 4, motor_dia], center=true);
    }
}

// Wheel well cutout
module wheel_well(side=1) {
    translate([side*(chassis_width/2), 0, wheel_dia/2-2]) {
        rotate([0, 90, 0])
            cylinder(d=wheel_dia+4, h=wheel_width+4, center=true, $fn=48);
    }
}

// Axle hole
module axle_hole() {
    rotate([0, 90, 0])
        cylinder(d=axle_dia+axle_clearance*2, h=chassis_width+10, center=true, $fn=24);
}

// Servo mount
module servo_mount() {
    difference() {
        // Mount platform
        cube([servo_width+4, servo_depth+4, servo_height+wall], center=true);

        // Servo cavity
        translate([0, 0, wall])
            cube([servo_width+0.5, servo_depth+0.5, servo_height+1], center=true);

        // Wire slot
        translate([0, -servo_depth/2-2, 0])
            cube([6, 5, servo_height+wall+1], center=true);
    }
}

// XIAO mount
module xiao_mount() {
    difference() {
        cube([xiao_width+4, xiao_depth+4, xiao_height+wall], center=true);
        translate([0, 0, wall])
            cube([xiao_width+0.5, xiao_depth+0.5, xiao_height+1], center=true);

        // USB-C access
        translate([xiao_width/2+2, 0, xiao_height/2])
            cube([5, 10, 4], center=true);
    }
}

// DRV8833 mount
module drv_mount() {
    difference() {
        cube([drv_width+3, drv_depth+3, drv_height+wall], center=true);
        translate([0, 0, wall])
            cube([drv_width+0.5, drv_depth+0.5, drv_height+1], center=true);
    }
}

// Battery compartment
module battery_compartment() {
    difference() {
        cube([batt_width+4, batt_depth+4, batt_height+wall], center=true);
        translate([0, 0, wall])
            cube([batt_width+0.5, batt_depth+0.5, batt_height+1], center=true);

        // Wire slot
        translate([batt_width/2+2, 0, 0])
            cube([5, 4, batt_height+wall+1], center=true);
    }
}

// ============================================
// CHASSIS BOTTOM
// ============================================

module chassis_bottom() {
    difference() {
        // Main body
        rounded_rect(chassis_width, chassis_length, chassis_height, r=3);

        // Interior hollow
        translate([0, 0, wall])
            rounded_rect(chassis_width-wall*2, chassis_length-wall*2, chassis_height, r=2);

        // Rear wheel wells
        translate([0, rear_axle_y, 0]) {
            wheel_well(1);
            wheel_well(-1);
        }

        // Front wheel wells
        translate([0, front_axle_y, 0]) {
            wheel_well(1);
            wheel_well(-1);
        }

        // Rear axle hole
        translate([0, rear_axle_y, wheel_dia/2-2])
            axle_hole();

        // Front axle hole
        translate([0, front_axle_y, wheel_dia/2-2])
            axle_hole();

        // USB-C access (side)
        translate([chassis_width/2, xiao_y, chassis_height/2])
            cube([wall*3, 12, 5], center=true);

        // Power switch hole
        translate([-chassis_width/2, batt_y, chassis_height/2])
            rotate([0, 90, 0])
                cylinder(d=5, h=wall*3, center=true, $fn=24);
    }

    // Motor mounts
    translate([motor_x_offset, motor_y, wall]) motor_mount();
    translate([-motor_x_offset, motor_y, wall]) mirror([1,0,0]) motor_mount();

    // Servo mount (front center)
    translate([0, servo_y, wall + servo_height/2 + wall/2])
        servo_mount();

    // Electronics mounts (stacked in center)
    translate([0, xiao_y, wall + xiao_height/2 + wall/2])
        xiao_mount();

    translate([0, drv_y, wall + drv_height/2 + wall/2])
        drv_mount();

    translate([0, batt_y, wall + batt_height/2 + wall/2])
        battery_compartment();

    // Screw posts for lid
    for (x = [-1, 1]) {
        translate([x*(chassis_width/2-4), 0, wall]) {
            difference() {
                cylinder(d=6, h=chassis_height-wall-2, $fn=24);
                cylinder(d=2, h=chassis_height, $fn=16);
            }
        }
    }
}

// ============================================
// CHASSIS TOP/LID
// ============================================

module chassis_top() {
    difference() {
        // Main lid
        rounded_rect(chassis_width, chassis_length, wall*2, r=3);

        // Screw holes
        for (x = [-1, 1]) {
            translate([x*(chassis_width/2-4), 0, 0])
                cylinder(d=2.5, h=wall*3, center=true, $fn=16);
        }

        // Ventilation slots
        for (i = [-2:2]) {
            translate([0, i*12, 0])
                rounded_rect(15, 6, wall*3, r=1);
        }

        // Servo arm slot
        translate([0, servo_y + 8, 0])
            cube([8, 15, wall*3], center=true);
    }

    // Lip to fit into chassis
    translate([0, 0, -wall]) {
        difference() {
            rounded_rect(chassis_width-wall*2-0.5, chassis_length-wall*2-0.5, wall, r=2);
            rounded_rect(chassis_width-wall*4, chassis_length-wall*4, wall+1, r=1);
        }
    }
}

// ============================================
// WHEEL (OPTIONAL - for visualization)
// ============================================

module wheel() {
    difference() {
        union() {
            // Tire
            rotate_extrude($fn=48)
                translate([wheel_dia/2-2, 0, 0])
                    circle(r=2, $fn=16);

            // Rim
            cylinder(d=wheel_dia-4, h=wheel_width-2, center=true, $fn=32);
        }

        // Axle hole
        cylinder(d=axle_dia, h=wheel_width+1, center=true, $fn=16);
    }
}

// ============================================
// RENDER OPTIONS
// ============================================

// Uncomment one of these to render:

// Full assembly with wheels (for visualization)
/*
chassis_bottom();
translate([0, 0, chassis_height + 3]) chassis_top();

// Wheels
color("DarkGray") {
    translate([chassis_width/2+wheel_width/2+1, rear_axle_y, wheel_dia/2-2])
        rotate([0, 90, 0]) wheel();
    translate([-chassis_width/2-wheel_width/2-1, rear_axle_y, wheel_dia/2-2])
        rotate([0, -90, 0]) wheel();
    translate([chassis_width/2+wheel_width/2+1, front_axle_y, wheel_dia/2-2])
        rotate([0, 90, 0]) wheel();
    translate([-chassis_width/2-wheel_width/2-1, front_axle_y, wheel_dia/2-2])
        rotate([0, -90, 0]) wheel();
}
*/

// Bottom only (for printing)
// chassis_bottom();

// Top only (for printing)
// chassis_top();

// Wheel only (for printing)
// wheel();

// Exploded view
chassis_bottom();
translate([0, 0, chassis_height + 15]) chassis_top();

color("DarkGray", 0.5) {
    translate([chassis_width/2+wheel_width/2+6, rear_axle_y, wheel_dia/2-2])
        rotate([0, 90, 0]) wheel();
    translate([-chassis_width/2-wheel_width/2-6, rear_axle_y, wheel_dia/2-2])
        rotate([0, -90, 0]) wheel();
    translate([chassis_width/2+wheel_width/2+6, front_axle_y, wheel_dia/2-2])
        rotate([0, 90, 0]) wheel();
    translate([-chassis_width/2-wheel_width/2-6, front_axle_y, wheel_dia/2-2])
        rotate([0, -90, 0]) wheel();
}

// ============================================
// PRINT INFO
// ============================================
// Recommended settings:
// - Layer height: 0.16-0.2mm
// - Infill: 15-20%
// - Supports: Yes (for wheel wells and motor mounts)
// - Material: PLA, PETG, or ABS
// - Print chassis bottom upside down for best wheel well quality
// - Print lid flat
// - Print wheels with 0.12mm layer height for smoothness
