import subprocess
import os

# ANSI color codes
GREEN = "\033[92m"
RED = "\033[91m"
RESET = "\033[0m"

# List of BLARGG'S TEST ROMS (update as needed)
roms = [
    # "test-roms/gb-test-roms/cpu_instrs/cpu_instrs.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/01-special.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/02-interrupts.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/03-op sp,hl.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/04-op r,imm.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/05-op rp.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/06-ld r,r.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/08-misc instrs.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/09-op r,r.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/10-bit ops.gb",
    "test-roms/gb-test-roms/cpu_instrs/individual/11-op a,(hl).gb"
]

results = {}

for rom in roms:
    print(f"Testing {rom}...")
    # Clean previous build/output
    subprocess.run(["make", "clean"], cwd=".")
    # Build the project
    build_result = subprocess.run(["make", "build"], cwd=".")
    if build_result.returncode != 0:
        results[rom] = "BUILD ERROR"
        continue

    # Run the program with the ROM path as argument
    run_result = subprocess.run(["./myprogram", rom, "--quiet"], cwd=".")
    if run_result.returncode != 0:
        results[rom] = "RUNTIME ERROR"
        continue

    # Check log.txt for pass/fail
    log_path = os.path.join("logs", "log.txt")
    try:
        with open(log_path, "r") as f:
            log = f.read()
            if "ALL TESTS PASSED" in log or "Passed" in log:
                results[rom] = "PASSED"
            else:
                results[rom] = "FAILED"
    except Exception as e:
        results[rom] = f"LOG ERROR: {e}"

print("\nTest Results:")
for rom, status in results.items():
    if status == "PASSED":
        color = GREEN
    elif status == "FAILED":
        color = RED
    else:
        color = RESET
    print(f"{rom}: {color}{status}{RESET}")
