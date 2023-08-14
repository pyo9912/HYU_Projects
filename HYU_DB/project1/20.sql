SELECT T.name, Count(*) AS '#CatchedPokemon'
FROM Trainer AS T, CatchedPokemon AS CP
WHERE T.id = CP.owner_id AND T.hometown = 'Sangnok City'
GROUP BY CP.owner_id
ORDER BY COUNT(*);