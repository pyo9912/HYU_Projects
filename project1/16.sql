SELECT COUNT(*) AS "#Pokemon in Water type, Electric type, Psychic type"
FROM Pokemon
WHERE type IN ('Water', 'Electric', 'Psychic');