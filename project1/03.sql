SELECT AVG(CP.level)
FROM Trainer AS T, CatchedPokemon AS CP, Pokemon AS P
WHERE CP.pid = P.id AND T.id = CP.owner_id
AND P.type = 'Electric' AND T.hometown = 'Sangnok City';