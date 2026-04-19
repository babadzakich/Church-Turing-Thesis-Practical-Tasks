from ortools.linear_solver import pywraplp
import argparse, sys


solver = pywraplp.Solver.CreateSolver("SCIP")


f = open(sys.argv[0])
n,m = (int(i) for i in f.readline().split())
modules = [solver.BoolVar(f"module {i}") for i in range(n)]
for j in range(m):
    s = f.readline().split()
    if s[0] == 'DEP':
        #print('dependent',int(s[1])-1,int(s[2])-1)
        solver.Add(modules[int(s[1])-1] <= modules[int(s[2])-1])

    if s[0] == 'CONFLICT':
        #print('conflict', int(s[1]) - 1, int(s[2]) - 1)
        solver.Add(modules[int(s[1])-1] + modules[int(s[2])-1] <= 1)
    else:
        #print('required', int(s[1]) - 1)
        solver.Add(modules[int(s[1])-1] == 1)
solver.Minimize(sum(modules))

status = solver.Solve()

o = open(sys.argv[1],'w')
if status == pywraplp.Solver.OPTIMAL:
    # print('Solution:')
    # print('Target Function = ', solver.Objective().Value())
    # print([modules[i].solution_value() for i in range(n)])
    for i in range(n):
        if modules[i].solution_value():
            print(f'{i+1}: ON')
        else:
            print(f'{i+1}: OFF')
else:
    print('IMPOSSIBLE')
    # print('IMPOSSIBLE')