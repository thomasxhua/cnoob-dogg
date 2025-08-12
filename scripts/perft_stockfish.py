import os
import subprocess
import re

stockfish_path = "./stockfish.exe"
cnoobdogg_path = "./dist/main.exe"

def open_process(cmd):
    process = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
    )
    print("> " + " ".join(cmd))
    return process

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
        if key in b_keys:
            b_val = b[key]
            if key in b_keys and a_val != b_val:
                diffs.append((key,a_val,b_val))
    return a_minus_b,b_minus_a,diffs

def stockfish_cnoobdogg(depth, moves=[], fen="", stockfish_verbose=False, cnoobdogg_verbose=False):
    stockfish = open_process([stockfish_path])
    cnoobdogg_cmd = ["perft" if (fen == "") else "perft-fen"] + [str(depth)]
    if fen != "":
        cnoobdogg_cmd.append(fen)
    cnoobdogg_cmd += moves
    cnoobdogg = open_process([cnoobdogg_path] + (["perft", str(depth)] if (fen == "") else ["perft-fen", str(depth), fen]) + moves)
    if (fen != ""):
        send_command(stockfish, f"position fen {fen} moves {" ".join(moves)}")
    else:
        send_command(stockfish, f"position startpos moves {" ".join(moves)}")
    send_command(stockfish, f"go perft {depth}")
    send_command(stockfish, "quit")
    stockfish_lines = get_lines(stockfish)
    cnoobdogg_lines = get_lines(cnoobdogg)
    if stockfish_verbose:
        print("----- stockfish execution -----")
        for line in stockfish_lines:
            print(line, end="")
        print("\n------------------------------")
    if cnoobdogg_verbose:
        print("----- cnoobdogg execution -----")
        for line in cnoobdogg_lines:
            print(line, end="")
        print("\n------------------------------")
    stockfish_dict = lines_to_dict(stockfish_lines)
    cnoobdogg_dict = lines_to_dict(cnoobdogg_lines)
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
    stockfish_total = sum(stockfish_dict.values())
    cnoobdogg_total = sum(cnoobdogg_dict.values())
    print(f"Move list: [{" ".join(moves)}]")
    print(f"Stockfish: {stockfish_total}")
    print(f"cnoobdogg: {cnoobdogg_total}")
    if (idx > 0):
        chosen_idx = int(input(f"Chase move [n]: "))
        stockfish_cnoobdogg(depth - 1, moves + [diffs[chosen_idx][0]], fen, stockfish_verbose, cnoobdogg_verbose)

def print_move(move):
    squares,count = move
    print(f"{squares}: {count}")

if not os.path.isfile(stockfish_path):
    print(f"Please provide a stockfish binary not found under {stockfish_path}.")

depth = 1
moves = []
fen   = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"

stockfish_cnoobdogg(depth, moves, fen)

