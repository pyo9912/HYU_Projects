import sys
import math

from sklearn import cluster, neighbors

class Point:
    def __init__(self, id, x, y):
        self.id = int(id)
        self.x = float(x)
        self.y = float(y)
        self.label = None
        ### label ###
        # None: not classified
        # 0: Noise
        # 1 ~ n : cluster id

def get_distance(a: Point, b: Point):
    return float(math.sqrt(math.pow(a.x-b.x, 2) + math.pow(a.y-b.y, 2)))

def get_neighbors(data, eps, obj):
    return [o for o in data if (o != obj and get_distance(o,obj) <= eps)]

def expand(data, neighbors, cluster_id, eps, min_pts):
    for obj in neighbors:
        if (obj.label == 0):
            obj.label = cluster_id

        if (obj.label == None):
            obj.label = cluster_id
            next_neighbors = get_neighbors(data, eps, obj)
            if (len(next_neighbors) >= min_pts):
                neighbors.extend(next_neighbors)

def clustering(data, n, eps, min_pts):
    cluster_id = 1
    
    for obj in data:
        # Skip classified object
        if (obj.label is not None):
            continue

        # For non-classified object
        # Get neighbors of the object
        neighbors = get_neighbors(data, eps, obj)
        # Cannot make cluster
        if (len(neighbors) < min_pts):
            obj.label = 0
            continue

        # Expand Cluster
        obj.label = cluster_id
        expand(data, neighbors, cluster_id, eps, min_pts)
        cluster_id += 1
    
    cluster_list = [[] for _ in range(0, cluster_id)]
    for obj in data:
        if (obj.label == 0):
            continue
        cluster_list[obj.label - 1].append(obj.id)
    cluster_list = cluster_list[:n]
    return cluster_list


def main():
    input_file = sys.argv[1]
    n = int(sys.argv[2])
    eps = float(sys.argv[3])
    min_pts = float(sys.argv[4])

    input_name = input_file.split(".")[0]

    data = []
    f = open(input_file,"r")
    for line in f.readlines():
        obj = line.split()
        point = Point(obj[0], obj[1], obj[2])
        data.append(point)

    dbscan = clustering(data, n, eps, min_pts)

    for index, output in enumerate(dbscan):
        output_name = input_name + "_cluster_" + str(index) + ".txt"
        with open(output_name, "w") as output_file:
            for i in output:
                output_file.write("%d\n"%i)
    

if __name__ == "__main__":
    main()
