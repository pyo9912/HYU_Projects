SELECT DISTINCT P.name
FROM Trainer AS T, CatchedPokemon AS CP, Pokemon AS P
WHERE P.id IN (
    SELECT P.id
    FROM Trainer AS T, CatchedPokemon AS CP, Pokemon AS P
    WHERE T.id = CP.owner_id AND CP.pid = P.id
    AND T.hometown = 'Sangnok City')
  AND P.id IN (
    SELECT P.id
    FROM Trainer AS T, CatchedPokemon AS CP, Pokemon AS P
    WHERE T.id = CP.owner_id AND CP.pid = P.id 
    AND T.hometown = 'Brown City')
 ORDER BY P.name;