SELECT T.hometown, T.name, AVG(CP.level)
FROM Trainer AS T, CatchedPokemon AS CP
WHERE T.id = CP.owner_id
GROUP BY CP.owner_id
ORDER BY AVG(CP.level);