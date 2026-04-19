---------- MODULE main----------
EXTENDS Integers, TLC

VARIABLES mtxs, scores, PCs

vars == << mtxs, PCs, scores >>

Init == /\ mtxs = [ mtx \in 1..5 |-> FALSE ]
        /\ scores = [ score \in 1..5 |-> 0 ]
        /\ PCs = [ pc \in 1..5 |-> "lock_first" ]

NextW(num) ==   \/  /\  PCs[num] = "lock_first"
                    /\  ~mtxs[num] /\ mtxs' = [mtxs EXCEPT ![num] = TRUE] /\ PCs' = [PCs EXCEPT ![num] = "lock_second"]
                    /\  scores' = scores
                \/  /\  PCs[num] = "lock_second"
                    /\  ~mtxs[((num + 1) % 5) + 1] /\ mtxs' = [mtxs EXCEPT ![((num + 1) % 5) + 1] = TRUE] /\ PCs' = [PCs EXCEPT ![num] = "incrementing"]
                    /\  scores' = scores
                \/  /\  PCs[num] = "incrementing"
                    /\  mtxs' = mtxs
                    /\  PCs' = [PCs EXCEPT ![num] = "unlock_first"] /\ mtxs' = mtxs /\ scores' = [scores EXCEPT ![num] = scores[num] + 1]
                \/  /\  PCs[num] = "unlock_first"
                    /\  mtxs' = mtxs
                    /\  PCs' = [PCs EXCEPT ![num] = "unlock_second"] /\ mtxs' = [mtxs EXCEPT ![num] = FALSE] /\ scores' = scores
                \/  /\  PCs[num] = "unlock_second"
                    /\  mtxs' = mtxs
                    /\  PCs' = [PCs EXCEPT ![num] = "lock_first"] /\ mtxs' = [mtxs EXCEPT ![((num + 1) % 5) + 1] = FALSE] /\ scores' = scores
                
Next == \E x \in 1..5: NextW(x)

Spec == Init /\ [][Next]_vars

================================
