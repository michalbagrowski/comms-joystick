// =====================================================
// Compact RX Enclosure - Parametric OpenSCAD Design
// =====================================================
// For: XIAO ESP32-C3 + DRV8833 + LiPo + Motors
// Target: Hot Wheels sized prototype testing
//
// To generate STL:
// 1. Open in OpenSCAD
// 2. Adjust parameters below for your components
// 3. Press F6 to render
// 4. Export as STL
// =====================================================

// ================== PARAMETERS ==================

// Main case dimensions (adjust to fit your chassis)
case_length = 50;      // X dimension (mm)
case_width = 28;       // Y dimension (mm)
case_height = 18;      // Z dimension (mm)
wall_thickness = 1.5;  // Wall thickness (mm)
corner_radius = 2;     // Corner rounding (mm)

// XIAO ESP32-C3 dimensions
xiao_length = 21;
xiao_width = 17.5;
xiao_height = 4;
xiao_x_offset = 2;     // Position from left wall
xiao_y_offset = 5;     // Position from front wall

// DRV8833 dimensions
drv_length = 15;
drv_width = 10;
drv_height = 3;
drv_x_offset = 25;     // Position from left wall
drv_y_offset = 9;      // Position from front wall

// LiPo battery dimensions (adjust for your battery)
battery_length = 25;   // 501230 or similar
battery_width = 12;
battery_height = 5;
battery_x_offset = 12; // Centered under electronics
battery_y_offset = 8;

// Motor mount dimensions
motor_diameter = 10;   // N20 or similar micro motor
motor_length = 20;     // Including gearbox
motor_spacing = 18;    // Distance between motor centers
motor_y_offset = 14;   // From front of case

// Servo slot dimensions
servo_width = 12;
servo_length = 25;
servo_height = 8;

// USB-C access hole
usb_width = 10;
usb_height = 4;

// Wire routing holes
wire_hole_diameter = 3;

// Lid attachment
lid_lip_height = 2;
lid_tolerance = 0.3;

// ================== MODULES ==================

// Rounded box helper
module rounded_box(length, width, height, radius) {
    hull() {
        translate([radius, radius, 0])
            cylinder(h=height, r=radius, $fn=32);
        translate([length-radius, radius, 0])
            cylinder(h=height, r=radius, $fn=32);
        translate([radius, width-radius, 0])
            cylinder(h=height, r=radius, $fn=32);
        translate([length-radius, width-radius, 0])
            cylinder(h=height, r=radius, $fn=32);
    }
}

// Motor mount cutout
module motor_mount(diameter, length) {
    rotate([0, 90, 0])
        cylinder(h=length+wall_thickness*2, d=diameter, $fn=32);
}

// XIAO cutout with USB access
module xiao_cutout() {
    // Main board space
    translate([xiao_x_offset, xiao_y_offset, wall_thickness])
        cube([xiao_length + 1, xiao_width + 1, xiao_height + 5]);

    // USB-C access hole (back of case)
    translate([xiao_x_offset + xiao_length/2 - usb_width/2, -1, wall_thickness + 2])
        cube([usb_width, wall_thickness + 2, usb_height]);
}

// DRV8833 cutout
module drv8833_cutout() {
    translate([drv_x_offset, drv_y_offset, wall_thickness])
        cube([drv_length + 1, drv_width + 1, drv_height + 5]);
}

// Battery compartment
module battery_compartment() {
    translate([battery_x_offset, battery_y_offset, wall_thickness])
        cube([battery_length + 1, battery_width + 1, battery_height + 1]);
}

// Wire routing holes
module wire_holes() {
    // Between battery and electronics
    translate([battery_x_offset + battery_length/2, battery_y_offset + battery_width/2, -1])
        cylinder(h=wall_thickness + 2, d=wire_hole_diameter, $fn=16);

    // Motor wire holes
    translate([5, motor_y_offset, case_height/2])
        rotate([0, 90, 0])
            cylinder(h=wall_thickness + 2, d=wire_hole_diameter, $fn=16);
    translate([case_length - wall_thickness - 1, motor_y_offset, case_height/2])
        rotate([0, 90, 0])
            cylinder(h=wall_thickness + 2, d=wire_hole_diameter, $fn=16);
}

// Servo slot
module servo_slot() {
    translate([case_length - servo_length - wall_thickness,
               case_width/2 - servo_width/2,
               wall_thickness])
        cube([servo_length + wall_thickness + 1, servo_width, servo_height + 5]);
}

// ================== MAIN CASE ==================

module case_bottom() {
    difference() {
        // Outer shell
        rounded_box(case_length, case_width, case_height - lid_lip_height, corner_radius);

        // Inner hollow
        translate([wall_thickness, wall_thickness, wall_thickness])
            rounded_box(case_length - wall_thickness*2,
                       case_width - wall_thickness*2,
                       case_height,
                       corner_radius - wall_thickness/2);

        // Component cutouts
        xiao_cutout();
        drv8833_cutout();
        battery_compartment();
        servo_slot();
        wire_holes();

        // Motor mounts (through holes on sides)
        translate([-1, motor_y_offset - motor_spacing/2, case_height/2])
            motor_mount(motor_diameter, motor_length);
        translate([-1, motor_y_offset + motor_spacing/2, case_height/2])
            motor_mount(motor_diameter, motor_length);
    }

    // Add mounting posts for XIAO
    translate([xiao_x_offset + 2, xiao_y_offset + 2, wall_thickness])
        cylinder(h=2, d=3, $fn=16);
    translate([xiao_x_offset + xiao_length - 2, xiao_y_offset + 2, wall_thickness])
        cylinder(h=2, d=3, $fn=16);
    translate([xiao_x_offset + 2, xiao_y_offset + xiao_width - 2, wall_thickness])
        cylinder(h=2, d=3, $fn=16);
    translate([xiao_x_offset + xiao_length - 2, xiao_y_offset + xiao_width - 2, wall_thickness])
        cylinder(h=2, d=3, $fn=16);
}

// ================== LID ==================

module case_lid() {
    difference() {
        union() {
            // Main lid
            rounded_box(case_length, case_width, wall_thickness, corner_radius);

            // Inner lip for snap fit
            translate([wall_thickness + lid_tolerance,
                      wall_thickness + lid_tolerance,
                      wall_thickness])
                rounded_box(case_length - wall_thickness*2 - lid_tolerance*2,
                           case_width - wall_thickness*2 - lid_tolerance*2,
                           lid_lip_height,
                           corner_radius - wall_thickness/2);
        }

        // Ventilation holes
        for (x = [10 : 8 : case_length - 10]) {
            for (y = [8 : 6 : case_width - 8]) {
                translate([x, y, -1])
                    cylinder(h=wall_thickness + 2, d=2, $fn=16);
            }
        }

        // Status LED window
        translate([xiao_x_offset + xiao_length/2, xiao_y_offset + xiao_width/2, -1])
            cylinder(h=wall_thickness + 2, d=4, $fn=16);
    }
}

// ================== RENDER ==================

// Uncomment one of these to render:

// Both parts side by side (for viewing)
case_bottom();
translate([case_length + 10, 0, 0])
    case_lid();

// Just the bottom case
// case_bottom();

// Just the lid
// case_lid();

// ================== PRINT SETTINGS ==================
//
// Recommended settings:
// - Layer height: 0.2mm
// - Infill: 20%
// - Supports: No (designed to print without)
// - Material: PLA or PETG
// - Print time: ~30 minutes per part
//
// =====================================================
