SELECT AVG(CP.level)
FROM Trainer AS T, CatchedPokemon AS CP
WHERE T.id = CP.owner_id AND T.name = 'Red';