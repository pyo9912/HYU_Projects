import sys
import itertools
import copy

# 입력받은 아이템 준에서 minimum support 값을 넘는 요소만 반환
def filtering(item_db, min_sup):
    frequent = dict(filter(lambda val: val[1] >= min_sup, item_db.items()))
    return frequent

# k 빈번 아이템에 없는 요소를 갖는 아이템 삭제
def pruning(candidate, prev_list, prev_cnt):
    for item in candidate:
        sub_item = list(itertools.combinations(item, prev_cnt))
        flag = True
        for sub in sub_item:
            if sub not in prev_list:
                flag = False
                break
        if flag is False:
            del candidate[candidate.index(item)]
    return candidate

# k+1 빈번 아이템의 후보 생성
def self_join(items, cnt):
    tmp_items = copy.deepcopy(items)
    candidate = []
    for set1 in tmp_items:
        for set2 in tmp_items:
            # 후보 선정을 원활히 하기 위해 union한 결과를 sort하여 후보 군에 추가
            tmp = sorted(list(set(set1).union(set(set2))))
            if len(tmp) == (cnt + 1) and tmp not in candidate:
                candidate.append(tmp)
    return pruning(candidate, tmp_items, cnt)

# Apriori 알고리즘
def apriori(transactions, frequent_items, minimum_support_cnt):
    cnt = 1
    k = 1
    while True:
        # 후보 아이템 생성
        # Step 1: self-join
        # Step 2: pruning
        candidate_items = self_join(frequent_items[k-1], cnt)
        # 다음 후보 아이템이 없는 경우 종료
        if len(candidate_items) == 0:
            break
        # 아이템과 빈도수를 저장하기 위한 딕셔너리
        item_cnt_dict = {}
        for item_set in transactions:
            for item in candidate_items:
                tmp_item = tuple(item)
                if set(tmp_item) <= set(item_set):
                    if tmp_item in item_cnt_dict:
                        item_cnt_dict[tmp_item] += 1
                    else:
                        item_cnt_dict[tmp_item] = 1
        frequent_items.append(filtering(item_cnt_dict, minimum_support_cnt))
        cnt += 1
        k += 1

def format(item_set):
    return '{'+','.join(map(str, item_set)) + '}'

# Association rule을 구하고 출력하는 함수
def association_rule(output_file, frequent_items, transactions):
    t_num = len(transactions)
    fo = open(output_file, "w")
    output = ""
    for i in range(len(frequent_items)): # 모든 행들을 순회하며
        for item in frequent_items[i]:      # 각 행에 대해서
            for size in range(1, i+1):      # 길이가 size인 조합 반환
                tmp_item = list(itertools.combinations(item, size))
                for tmp in tmp_item:        # 생성된 조합들로 이루어진 집합의 모든 부분집합들을 비교
                    result_item = list(item)
                    result_item = tuple([x for x in result_item if x not in tmp])
                    # X, Y 두 아이템이 같이 나오는 비율
                    support = (frequent_items[i][item]/t_num) * 100
                    # X가 나온 경우 X와 Y가 같이 나오는 비율
                    item_set = format(tmp)
                    associative_item_set = format(result_item)
                    confidence = (frequent_items[i][item]/frequent_items[len(set(tmp))-1][tmp]) * 100
                    output += f"{item_set}\t{associative_item_set}\t"+str("%.2f"%round(support,2))+"\t"+str("%.2f"%round(confidence,2))+"\n"
    fo.write(output)
    fo.close()

def main():
    minimum_support = int(sys.argv[1])
    input_file = sys.argv[2]
    output_file = sys.argv[3]

    transactions = []
    input_items = {}
    frequent_items = []

    # DB의 transaction 정보들을 transactions table에 담는다
    with open(input_file, "r") as f:
        while True:
            line = f.readline()
            if not line:
                break
            str = line.split("\n")
            # 각 줄에 있는 아이템을 입력받아 transaction에 넣어준다
            items = list(map(int, str[0].split('\t')))
            transactions.append(items)
            # transaction의 아이템 입력받기
            for item in items:
                item = tuple([item])
                if item in input_items:
                    input_items[item] += 1
                else:
                    input_items[item] = 1
    f.close()
    # 퍼센트로 입력받은 minimum support 값을 count로 변환
    minimum_support_cnt = float(minimum_support / 100) * len(transactions)
    # 빈번 아이템 필터링
    frequent_items.append(filtering(input_items, minimum_support_cnt))

    apriori(transactions, frequent_items, minimum_support_cnt)
    association_rule(output_file, frequent_items, transactions)


if __name__ == "__main__":
    main()