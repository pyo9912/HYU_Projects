SELECT COUNT(DISTINCT P.type) AS '#Type'
FROM Trainer AS T, Gym AS G, CatchedPokemon AS CP, Pokemon AS P
WHERE T.id = G.leader_id AND T.id = CP.owner_id AND CP.pid = P.id
AND G.city = 'Sangnok City';