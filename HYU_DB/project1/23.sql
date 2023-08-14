SELECT DISTINCT T.name
FROM Trainer AS T, CatchedPokemon AS CP
WHERE T.id = CP.owner_id AND CP.level <= 10
ORDER BY T.name;