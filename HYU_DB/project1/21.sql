SELECT T.name, COUNT(*) AS '#CatchedPokemon'
FROM Trainer AS T, Gym AS G, CatchedPokemon AS CP
WHERE T.id = G.leader_id AND T.id = CP.owner_id
GROUP BY CP.owner_id
ORDER BY T.name;