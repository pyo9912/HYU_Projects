SELECT T.name, AVG(CP.level)
FROM Gym AS G, Trainer AS T, CatchedPokemon AS CP
WHERE T.id = G.leader_id AND T.id = CP.owner_id
GROUP BY G.leader_id
ORDER BY T.name;