SELECT name
FROM Trainer
WHERE name NOT IN (
  SELECT name
  FROM Trainer AS T, Gym As G
  WHERE T.id = G.leader_id
  )
 ORDER BY name;