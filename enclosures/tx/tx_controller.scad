// ============================================
// TX Controller Enclosure - ESP32 Joystick
// ============================================
// Parametric design for 3D printing
// Holds: 2x HW-504 joysticks, ESP32-C3, 1.9" OLED, LiPo battery
//
// Author: Generated for ESP32 Joystick Project
// Version: 1.0
// License: MIT

// ============================================
// PARAMETERS - Adjust these for your components
// ============================================

// Wall thickness
wall = 2.5;

// Component dimensions (measure your actual parts!)
// HW-504 Joystick Module
joy_width = 26;      // X dimension
joy_depth = 34;      // Y dimension
joy_height = 32;     // Height including stick
joy_hole_dia = 22;   // Hole for joystick shaft
joy_mount_hole = 3;  // Mounting screw holes

// ESP32-C3 Super Mini
esp_width = 22;
esp_depth = 18;
esp_height = 4;

// 1.9" OLED Display (SH1106)
oled_width = 35;
oled_depth = 18;
oled_height = 3;
oled_screen_w = 30;  // Visible screen area
oled_screen_h = 15;

// LiPo Battery (adjust to your battery)
// Default: 502535 size (500mAh)
batt_width = 35;
batt_depth = 25;
batt_height = 5;

// USB-C port dimensions
usbc_width = 9;
usbc_height = 3.5;

// Joystick spacing
joy_spacing = 70;    // Center to center distance

// ============================================
// CALCULATED DIMENSIONS
// ============================================

// Case interior dimensions
interior_width = joy_spacing + joy_width + 20;
interior_depth = max(joy_depth + 10, batt_depth + esp_depth + 15);
interior_height = joy_height + batt_height + 5;

// Case exterior dimensions
case_width = interior_width + 2*wall;
case_depth = interior_depth + 2*wall;
case_height = interior_height + 2*wall;

// Component positions
joy1_x = -joy_spacing/2;
joy2_x = joy_spacing/2;
joy_y = interior_depth/2 - joy_depth/2 - 5;

oled_x = 0;
oled_y = -interior_depth/2 + oled_depth/2 + 5;

esp_x = -interior_width/4;
esp_y = -interior_depth/2 + esp_depth/2 + 5;

batt_x = interior_width/4;
batt_y = -interior_depth/2 + batt_depth/2 + 5;

// ============================================
// MODULES
// ============================================

// Rounded box
module rounded_box(w, d, h, r=3) {
    hull() {
        for (x = [-1, 1], y = [-1, 1]) {
            translate([x*(w/2-r), y*(d/2-r), 0])
                cylinder(h=h, r=r, $fn=32);
        }
    }
}

// Joystick cutout
module joystick_cutout() {
    // Main shaft hole
    cylinder(d=joy_hole_dia, h=wall*3, center=true, $fn=48);

    // Mounting holes (4 corners)
    for (x = [-1, 1], y = [-1, 1]) {
        translate([x*(joy_width/2-4), y*(joy_depth/2-4), 0])
            cylinder(d=joy_mount_hole, h=wall*3, center=true, $fn=16);
    }
}

// Joystick mount posts
module joystick_posts() {
    post_height = interior_height - joy_height + 5;
    for (x = [-1, 1], y = [-1, 1]) {
        translate([x*(joy_width/2-4), y*(joy_depth/2-4), -interior_height/2]) {
            difference() {
                cylinder(d=joy_mount_hole+4, h=post_height, $fn=16);
                cylinder(d=joy_mount_hole-0.5, h=post_height+1, $fn=16);
            }
        }
    }
}

// OLED display cutout
module oled_cutout() {
    // Screen window
    cube([oled_screen_w, oled_screen_h, wall*3], center=true);

    // Bezel recess
    translate([0, 0, wall/2])
        cube([oled_width+1, oled_depth+1, wall], center=true);
}

// USB-C port cutout
module usbc_cutout() {
    hull() {
        for (x = [-1, 1]) {
            translate([x*(usbc_width/2-usbc_height/2), 0, 0])
                cylinder(d=usbc_height, h=wall*3, center=true, $fn=16);
        }
    }
}

// Battery compartment
module battery_compartment() {
    translate([0, 0, -interior_height/2]) {
        cube([batt_width+2, batt_depth+2, batt_height+2], center=true);
    }
}

// ESP32 mount
module esp_mount() {
    // Simple platform with retaining clips
    translate([0, 0, -interior_height/2 + batt_height + 2]) {
        difference() {
            cube([esp_width+6, esp_depth+6, 3], center=true);
            cube([esp_width+0.5, esp_depth+0.5, 4], center=true);
        }
    }
}

// ============================================
// MAIN CASE - BOTTOM
// ============================================

module case_bottom() {
    difference() {
        // Outer shell
        rounded_box(case_width, case_depth, case_height/2 + wall, r=5);

        // Interior cavity
        translate([0, 0, wall])
            rounded_box(interior_width, interior_depth, case_height/2 + 1, r=3);

        // USB-C port on side
        translate([case_width/2, esp_y, wall + batt_height + esp_height/2])
            rotate([0, 90, 0])
                usbc_cutout();

        // Power switch hole (optional)
        translate([-case_width/2, 0, case_height/4])
            rotate([0, 90, 0])
                cylinder(d=6, h=wall*3, center=true, $fn=24);
    }

    // Joystick mount posts
    translate([joy1_x, joy_y, 0]) joystick_posts();
    translate([joy2_x, joy_y, 0]) joystick_posts();

    // Battery retaining walls
    translate([batt_x, batt_y, -interior_height/4 + wall]) {
        for (x = [-1, 1]) {
            translate([x*(batt_width/2+1), 0, 0])
                cube([2, batt_depth-5, batt_height+4], center=true);
        }
    }

    // Screw bosses for lid
    for (x = [-1, 1], y = [-1, 1]) {
        translate([x*(interior_width/2-5), y*(interior_depth/2-5), 0]) {
            difference() {
                cylinder(d=8, h=case_height/2, $fn=24);
                translate([0, 0, case_height/4])
                    cylinder(d=2.5, h=case_height/4+1, $fn=16);
            }
        }
    }
}

// ============================================
// MAIN CASE - TOP/LID
// ============================================

module case_top() {
    difference() {
        // Outer shell
        rounded_box(case_width, case_depth, case_height/2, r=5);

        // Interior cavity (lip to fit into bottom)
        translate([0, 0, -1])
            rounded_box(interior_width+1, interior_depth+1, case_height/2-wall+1, r=3);

        // Joystick holes
        translate([joy1_x, joy_y, 0]) joystick_cutout();
        translate([joy2_x, joy_y, 0]) joystick_cutout();

        // OLED window
        translate([oled_x, oled_y, 0]) oled_cutout();

        // Screw holes
        for (x = [-1, 1], y = [-1, 1]) {
            translate([x*(interior_width/2-5), y*(interior_depth/2-5), 0])
                cylinder(d=3, h=case_height, center=true, $fn=16);
        }

        // Ventilation slots
        for (i = [-2:2]) {
            translate([i*12, -case_depth/2+15, case_height/4-wall])
                cube([8, 20, 2], center=true);
        }
    }

    // OLED retaining frame
    translate([oled_x, oled_y, -case_height/4+wall]) {
        difference() {
            cube([oled_width+4, oled_depth+4, oled_height+2], center=true);
            cube([oled_width+0.5, oled_depth+0.5, oled_height+3], center=true);
        }
    }
}

// ============================================
// RENDER OPTIONS
// ============================================

// Uncomment one of these to render:

// Full assembly (for visualization)
// translate([0, 0, case_height/2]) case_bottom();
// translate([0, 0, case_height + 5]) case_top();

// Bottom only (for printing)
// case_bottom();

// Top only (for printing)
// rotate([180, 0, 0]) case_top();

// Exploded view
translate([0, 0, 0]) case_bottom();
translate([0, 0, case_height/2 + 20]) case_top();

// ============================================
// PRINT INFO
// ============================================
// Recommended settings:
// - Layer height: 0.2mm
// - Infill: 20%
// - Supports: Yes (for joystick holes and USB port)
// - Material: PLA or PETG
// - Print bottom facing down
// - Print top facing up (or flipped with supports)
