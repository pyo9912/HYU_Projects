SELECT T.name, MAX(CP.level)
FROM Trainer AS T, CatchedPokemon AS CP
WHERE T.id = CP.owner_id
GROUP BY CP.owner_id
HAVING COUNT(*) >= 4
ORDER BY T.name;