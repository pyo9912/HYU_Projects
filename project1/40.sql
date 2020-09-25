SELECT T.hometown, CP.nickname
FROM Trainer AS T, CatchedPokemon AS CP, (
  SELECT Trainer.hometown, MAX(SUB1.M) AS LV
  FROM Trainer,
  (
  SELECT owner_id, MAX(level) AS M
  FROM CatchedPokemon
  GROUP BY owner_id
  ) AS SUB1
  WHERE Trainer.id = SUB1.owner_id
  GROUP BY Trainer.hometown
)AS SUB2
WHERE T.hometown = SUB2.hometown AND T.id = CP.owner_id
AND CP.level = SUB2.LV
GROUP BY T.hometown
ORDER BY T.hometown;