from ortools.linear_solver import pywraplp

solver = pywraplp.Solver.CreateSolver("SCIP")


f = open('input.txt')
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

o = open('output.txt','w')
if status == pywraplp.Solver.OPTIMAL:
    # print('Solution:')
    # print('Target Function = ', solver.Objective().Value())
    # print([modules[i].solution_value() for i in range(n)])
    for i in range(n):
        if modules[i].solution_value():
            o.write(f'{i+1}: ON\n')
        else:
            o.write(f'{i+1}: OFF\n')
else:
    o.write('IMPOSSIBLE')
    # print('IMPOSSIBLE')