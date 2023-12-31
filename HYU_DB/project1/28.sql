SELECT T.name, AVG(CP.level)
FROM Trainer AS T, CatchedPokemon AS CP, Pokemon AS P
WHERE T.id = CP.owner_id AND CP.pid = P.id AND
P.type IN ('Normal', 'Electric')
GROUP BY T.id
ORDER BY AVG(CP.level);