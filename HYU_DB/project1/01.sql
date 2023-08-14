SELECT T.name AS 'Trainer name'
FROM Trainer AS T, CatchedPokemon AS CP
WHERE T.id = CP.owner_id
GROUP BY CP.owner_id
HAVING COUNT(*) >= 3
ORDER BY COUNT(*) DESC;
