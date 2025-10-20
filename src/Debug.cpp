#include <iostream>
#include <cstdio>
#include <sstream>
#include <iomanip>

#include "main.h"
#include "CPU.h"
#include "Debug.h"
#include "Memory.h"


Debug::Debug() {
    instr_cnt = 0;
    num_steps_left = 0;
    run = false;
    tgt_pc.valid = 0;
    tgt_instr = 0xd3;   // Init to some illegal value that I would never target
    bp_info.breakpoint = false;
    bp_info.disable_breakpoints = false;

    perf.num_main_loops = 0;
    perf.frame_cpu_time_ms = 0.0;
    perf.frame_ppu_time_ms = 0.0;
    perf.frame_mmio_time_ms = 0.0;
    perf.frame_lcd_time_ms = 0.0;
    perf.frame_sdl_events_time_ms = 0.0;
    perf.frame_debugger_time_ms = 0.0;
    perf.frame_interrupt_time_ms = 0.0;
    perf.frame_other_time_ms = 0.0;
    perf.csv_logging_enabled = true;  // Enable CSV logging by default

    perf.frame_start_time = std::chrono::high_resolution_clock::now();
    perf.last_section_time = std::chrono::high_resolution_clock::now();
}

void parse_dbg_cmd(std::string &dbg_cmd, std::vector<std::string> &words) {
    // std::string dbg_cmd;
    if (!std::getline(std::cin, dbg_cmd)) {
        printx("\nExiting...\n");
        std::exit(0);   // Exit program
    }

    std::istringstream iss(dbg_cmd);
    
    std::string word;

    while (iss >> word) {
        words.push_back(word);
    }
}

void Debug::set_breakpoint(std::string msg) {
    bp_info.breakpoint = true;
    bp_info.msg = msg;
}


// Move this to a separate file so I can keep track of all the global vars easier
// Can also use the vars to interract with the program automatically
// num_steps_left, breakpoint, run, clk_cnt, tg_instr etc etc
void Debug::debugger_break(CPU &cpu) {
    instr_cnt++;

    if (DEBUGGER == false) return;

    bool exit_debugger = false;
    bool break_execution = true;
    // If run, dont break execution (priority = 4)
    if (run) {
        break_execution = false;
    }
    // If requested step count is reached, break execution (priority = 3)
    if (num_steps_left > 0) {
        break_execution = false;
        num_steps_left--;
    }
    // If requested instruction is found, break execution (priority = 2)
    if (tgt_instr != 0xd3) {
        if (mem->get(cpu.rf.get(PC)) == tgt_instr) {
            break_execution = true;
            tgt_instr = 0xd3;
        } else {
            break_execution = false;
        }
    }
    if (tgt_pc.valid) {
        if (cpu.rf.get(PC) == tgt_pc.pc) {
            break_execution = true;
            tgt_pc.valid = true;
        } else {
            break_execution = false;
        }
    }
    // If breakpoint is triggered in code, break execution (priority = 1)
    if (!bp_info.disable_breakpoints && bp_info.breakpoint) {
        printx ("Hit breakpoint! \"%s\" @ mcycle=%0lu\n", bp_info.msg.c_str(), mcycle_cnt);
        break_execution = true;
        bp_info.breakpoint = false;
        num_steps_left = 0;
        tgt_instr = 0xd3;
        tgt_pc.valid = false;
    }


    // Break CPU execution for debugging
    if (break_execution) {
        do  {
            PRINT_REGS_EN = true;
            printx (">> ");
            std::string dbg_cmd;
            std::vector<std::string> words;
            parse_dbg_cmd(dbg_cmd, words);
            if (words.empty()) continue;

            if (dbg_cmd[0] == 'h') {    // help
                printx ("Debug commands: 'sx'(step); 'r'(run); 'p'(print regs); 'mxxxx'(Read mem addr)\n");
            } else if (std::tolower(dbg_cmd[0]) == 's') {
                // prints state of regs and PC THAT WERE EXECUTED
                if (std::islower(dbg_cmd[0])) PRINT_REGS_EN = false; // Step quietly (dont print regs with each step)
                
                if (dbg_cmd.size() > 1) {
                    if (dbg_cmd[1] == 'i') {
                        // step to next occurence of instruction:
                        tgt_instr = std::stoi(dbg_cmd.substr(2), nullptr, 16);
                    } else if (dbg_cmd[1] == 't') {
                        // Step to right before line number xxxx
                        int step_input = std::stoi(dbg_cmd.substr(2));
                        num_steps_left = step_input - instr_cnt - 1;
                    } else if (dbg_cmd[1] == 'p' && dbg_cmd[2] == 'c') {
                        // Step to PC == 0x____
                        tgt_pc.pc = std::stoi(dbg_cmd.substr(3), nullptr, 16);
                        tgt_pc.valid = true;
                    } else {
                        // Step xxxx lines
                        int step_input = std::stoi(dbg_cmd.substr(1));
                        num_steps_left = (step_input == 0) ? 0 : (step_input - 1);
                    }
                }
                exit_debugger = true;
            
            } else if (dbg_cmd[0] == 'e') {
                disable_prints = !disable_prints; // enable prints
                printx ("disable_prints <= %0d\n", disable_prints);
            } else if (dbg_cmd[0] == 'p') {
                // prints state of regs and pc ABOUT TO BE EXECUTED
                cpu.rf.print_regs();
            } else if (dbg_cmd[0] == 'm') {
                if (dbg_cmd.size() == 5) {
                    int addr = std::stoi(dbg_cmd.substr(1), nullptr, 16);
                    uint16_t val = mem->get(addr);
                    printx ("mem[0x%0x] = 0x%0x\n", addr, val);
                } else if (dbg_cmd[1] == 'd' && dbg_cmd.size() == 6) {
                    // Memory Dump
                    int addr = std::stoi(dbg_cmd.substr(2), nullptr, 16);
                    for (int i = (addr-20); i < (addr+20); i++) {
                        printx ("mem[0x%0x]=0x%0x, ", i, mem->get(i));
                        if ((i - (addr-20)) % 10 == 9) {
                            printx ("\n");
                        }
                    }
                }
                else {
                    printx ("Please provide a 4 digit hex address with m/md: mxxxx\n");
                }
            } else if (dbg_cmd[0] == 'd') {
                // if (dbg_cmd[1] == '0') printx ("debug0 = 0x%0x\n", cpu.rf.debug0);
                // else if (dbg_cmd[1] == '1') printx ("debug1 = 0x%0x\n", cpu.rf.debug1);
                // else if (dbg_cmd[1] == '2') printx ("debug2 = 0x%0x\n", cpu.rf.debug2);
            } else if (dbg_cmd[0] == 'i' && dbg_cmd[1] == 'd') {
                disable_interrupts = !disable_interrupts;
                printx ("disable_interrupts <= %0d\n", disable_interrupts);
            } else if (dbg_cmd[0] == 'b' && dbg_cmd[1] == 'd') {
                bp_info.disable_breakpoints = !bp_info.disable_breakpoints;
                printx ("disable_breakpoints <= %0d\n", bp_info.disable_breakpoints);
            } else if (dbg_cmd[0] == 'r') {
                // DEBUGGER = false;   // disable debugger for rest of the run
                exit_debugger = true;
                run = true;
            } else if (dbg_cmd[0] == 'l') {
                // Line Count
                printx("Instr count = %lu\n", instr_cnt);
            }
            
        } while (exit_debugger == false);

    }
}


// CSV performance logging implementation
void Debug::init_csv_logging() {
    if (!perf.csv_logging_enabled) return;

    write_csv_header();
}

void Debug::start_frame_timing() {
    if (!perf.csv_logging_enabled) return;

    perf.frame_start_time = std::chrono::high_resolution_clock::now();
    perf.frame_cpu_time_ms = 0.0;
    perf.frame_ppu_time_ms = 0.0;
    perf.frame_mmio_time_ms = 0.0;
    perf.frame_lcd_time_ms = 0.0;
    perf.frame_sdl_events_time_ms = 0.0;
    perf.frame_debugger_time_ms = 0.0;
    perf.frame_interrupt_time_ms = 0.0;
    perf.frame_other_time_ms = 0.0;

    perf.last_section_time = perf.frame_start_time;
}

void Debug::log_component_timing(const std::string& component, double time_ms) {
    if (!perf.csv_logging_enabled) return;

    if (component == "cpu") {
        perf.frame_cpu_time_ms += time_ms;
    } else if (component == "ppu") {
        perf.frame_ppu_time_ms += time_ms;
    } else if (component == "mmio") {
        perf.frame_mmio_time_ms += time_ms;
    } else if (component == "lcd") {
        perf.frame_lcd_time_ms += time_ms;
    } else if (component == "sdl_events") {
        perf.frame_sdl_events_time_ms += time_ms;
    } else if (component == "debugger") {
        perf.frame_debugger_time_ms += time_ms;
    } else if (component == "interrupt") {
        perf.frame_interrupt_time_ms += time_ms;
    } else if (component == "other") {
        perf.frame_other_time_ms += time_ms;
    }
}

void Debug::finalize_frame_timing() {
    if (!perf.csv_logging_enabled) return;

    auto frame_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration<double, std::milli>(frame_end_time - perf.frame_start_time);

    FramePerfData frame_data;
    frame_data.cpu_time_ms = perf.frame_cpu_time_ms;
    frame_data.ppu_time_ms = perf.frame_ppu_time_ms;
    frame_data.mmio_time_ms = perf.frame_mmio_time_ms;
    frame_data.lcd_time_ms = perf.frame_lcd_time_ms;
    frame_data.sdl_events_time_ms = perf.frame_sdl_events_time_ms;
    frame_data.debugger_time_ms = perf.frame_debugger_time_ms;
    frame_data.interrupt_time_ms = perf.frame_interrupt_time_ms;
    frame_data.other_time_ms = perf.frame_other_time_ms;
    frame_data.total_frame_time_ms = total_duration.count();

    write_csv_row(frame_data);
}

void Debug::write_csv_header() {
    std::ofstream perf_csv("logs/performance.csv", std::ios::out);
    if (perf_csv.is_open()) {
        perf_csv << "frame_number,cpu_time_ms,ppu_time_ms,mmio_time_ms,lcd_time_ms,sdl_events_time_ms,debugger_time_ms,interrupt_time_ms,other_time_ms,total_frame_time_ms\n";
        perf_csv.close();
    }
}

void Debug::write_csv_row(const FramePerfData& data) {
    std::ofstream perf_csv("logs/performance.csv", std::ios::app);
    if (perf_csv.is_open()) {
        perf_csv << frame_cnt << ","
                << std::fixed << std::setprecision(4)
                << data.cpu_time_ms << ","
                << data.ppu_time_ms << ","
                << data.mmio_time_ms << ","
                << data.lcd_time_ms << ","
                << data.sdl_events_time_ms << ","
                << data.debugger_time_ms << ","
                << data.interrupt_time_ms << ","
                << data.other_time_ms << ","
                << data.total_frame_time_ms << "\n";
        perf_csv.close();
    }
}

// Detailed section timing methods
void Debug::start_section_timing() {
    if (!perf.csv_logging_enabled) return;
    perf.last_section_time = std::chrono::high_resolution_clock::now();
}

void Debug::end_section_timing(const std::string& section_name) {
    if (!perf.csv_logging_enabled) return;

    auto current_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(current_time - perf.last_section_time);

    log_component_timing(section_name, duration.count());
    perf.last_section_time = current_time;
}