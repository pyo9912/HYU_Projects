SELECT COUNT(*)
FROM Pokemon AS P
WHERE P.name IN (
  SELECT P.name
  FROM Trainer AS T, CatchedPokemon AS CP, Pokemon AS P
  WHERE T.id = CP.owner_id AND CP.pid = P.id
  AND T.hometown = 'Sangnok City'
  GROUP BY P.name
  );