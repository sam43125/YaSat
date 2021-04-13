#!/usr/bin/env python3
import os, sys, time

if __name__ == "__main__":

    if len(sys.argv) < 2 or not os.path.isdir(sys.argv[1]):
        print("Usage: " + sys.argv[0] + " </path/to/cnf_folder>")
        exit()

    os.system("make")

    filenames = os.listdir(sys.argv[1])

    start_time = time.time()

    for filename in filenames:

        fullpath = os.path.join(sys.argv[1], filename)

        if not os.path.isfile(fullpath) or fullpath[-4:] != ".cnf":
            continue

        cnf_filename = fullpath
        sat_filename = cnf_filename[:-4] + ".sat"
        os.system(f"sed -i 's/^[%0]$/c \\0/g' {cnf_filename}")

        cmd = f"./yasat {cnf_filename}"
        print(cmd, flush=True)
        os.system(cmd)

        with open(cnf_filename, "r") as cnf, \
             open(sat_filename, "r") as sat:

            clauses = list(filter(lambda s: s[0] not in ('p', 'c', '\n'), cnf.readlines()))
            clauses = list(map(lambda s: list(map(int, s.strip().split(" "))), clauses))

            assignments = sat.readlines()
            if "UNSAT" in assignments[0]:
                print("UNSAT")
                exit()
            assignments = set(map(int, assignments[1][2:].strip().split(" ")))
            assignments.remove(0)

            for clause in clauses:
                for var in clause:
                    if var in assignments:
                        break
                else:
                    print("Incorrect answer")
                    exit()

            print("Correct answer")

    print("--- %s seconds ---" % (time.time() - start_time))
