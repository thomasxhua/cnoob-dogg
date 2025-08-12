import os
import subprocess
import re

stockfish_path = "./stockfish.exe"
cnoobdogg_path = "./dist/main.exe"

def open_process(cmd):
    return subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
    )

def close_process(process):
    process.stdin.close()
    process.stdout.close()
    process.stderr.close()
    process.wait()

def send_command(process, cmd):
    print(f"> {cmd}")
    process.stdin.write(cmd + "\n")
    process.stdin.flush()

def get_lines(process):
    lines = []
    while True:
        line = process.stdout.readline()
        if not line:
            break
        lines.append(line)
    return lines

def print_lines(lines):
    for line in lines:
        print(line, end="")

def lines_to_dict(lines):
    pattern = r"^([a-z0-9]{4}): (\d+)$"
    moves = {}
    for line in lines:
        match = re.match(pattern, line)
        if match:
            squares = match.group(1)
            count   = int(match.group(2))
            moves[squares] = count
    return moves

def compare_dicts(a,b):
    a_keys    = set(a.keys())
    b_keys    = set(b.keys())
    a_minus_b = [(key,a[key]) for key in (a_keys - b_keys)]
    b_minus_a = [(key,b[key]) for key in (b_keys - a_keys)]
    diffs = []
    for key,val in a.items():
        a_val = a[key]
        b_val = b[key]
        if key in b_keys and a_val != b_val:
            diffs.append((key,a_val,b_val))
    return a_minus_b,b_minus_a,diffs

def stockfish_cnoobdogg(depth, moves):
    stockfish = open_process([stockfish_path])
    cnoobdogg = open_process([cnoobdogg_path, "perft", str(depth)] + moves)
    send_command(stockfish, f"position startpos moves {" ".join(moves)}")
    send_command(stockfish, f"go perft {depth}")
    send_command(stockfish, "quit")
    stockfish_dict = lines_to_dict(get_lines(stockfish))
    cnoobdogg_dict = lines_to_dict(get_lines(cnoobdogg))
    stockfish_uniques,cnoobdogg_uniques,diffs = compare_dicts(stockfish_dict, cnoobdogg_dict)
    print("===   Stockfish Uniques   ===")
    for move in stockfish_uniques:
        print_move(move)
    print("===   cnoobdogg Uniques   ===")
    for move in cnoobdogg_uniques:
        print_move(move)
    print("=== Stockfish - cnoobdogg ===")
    idx = 0
    for squares,stockfish_count,cnoobdogg_count in diffs:
        print(f"[{idx}] {squares}: {stockfish_count} - {cnoobdogg_count} = {stockfish_count-cnoobdogg_count}")
        idx += 1
    close_process(stockfish)
    close_process(cnoobdogg)
    print(f"Move list: [{" ".join(moves)}]")
    if idx != 0:
        chosen_idx = int(input(f"Chase move [n]: "))
        stockfish_cnoobdogg(depth - 1, moves + [diffs[chosen_idx][0]])

def print_move(move):
    squares,count = move
    print(f"{squares}: {count}")

if not os.path.isfile(stockfish_path):
    print(f"Please provide a stockfish binary not found under {stockfish_path}.")

depth = 5
moves = ["c2c3"]

stockfish_cnoobdogg(depth, moves)

# [('c2c3', 5417640, 5417741), ('e2e3', 9726018, 9726366), ('a2a4', 5363555, 5363574), ('b2b4', 5293555, 5293580), ('c2c4', 5866666, 5866804), ('d2d4', 8879566, 8879595), ('e2e4', 9771632, 9772009), ('f2f4', 4890429, 4890462), ('g2g4', 5239875, 5239902), ('h2h4', 5385554, 5385569)]
