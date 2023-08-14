SELECT P.name, P.id
FROM Pokemon AS P, Trainer AS T, CatchedPokemon AS CP
WHERE P.id = CP.pid AND CP.owner_id = T.id AND T.hometown = 'Sangnok City'
ORDER BY P.id;