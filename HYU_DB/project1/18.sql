SELECT AVG(CP.level)
FROM Trainer AS T, Gym AS G, CatchedPokemon AS CP
WHERE T.id = G.leader_id AND T.id = CP.owner_id;