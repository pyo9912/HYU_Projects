import sys
import pandas as pd
import math

class Node:
    def __init__(self, attr, isLeaf = False, label = None):
        self.attr = attr        # DB의 카테고리 (ex: attr 1, attr 2, ... , attr n)
        self.isleaf = isLeaf   # 노드 속성 (ex: leaf node/internal node)
        self.label = label      # 데이터의 최종 분류 기준 (ex: yes/no)
        self.child = dict()

class DecisionTree:
    '''
    Decision tree을 초기화한다.
    '''
    def __init__(self, df):
        # Data frame의 마지막 column에 label 정보가 담겨있음
        self.label = df.columns[-1]
        # Data frame의 attribute와 attribute의 value set들 추출하기
        self.attrs = {attr: df[attr].unique() for attr in df.columns if attr!=self.label}
        # Decision tree의 root 노드
        self.root = self.create_dt(df)

    '''
    Data frame의 label 값들을 분류하여 각 값이 나올 확률을 구한다.
    '''
    def probability(self, df):
        values = df[self.label]
        total_cnt = len(values)
        return [float(value)/float(total_cnt) for value in values.value_counts()]

    '''
    확률을 받아와 entropy를 구한다.
    '''
    def entropy(self, prob):
        return sum(-1.0 * (p *math.log(p + 1e-15,2)) for p in prob if p)

    '''
    특정 attribute를 인자로 받아 해당 attribute의 information gain을 구한다.
    '''
    def information_gain(self, df, attr):
        info_D = self.entropy(self.probability(df))
        info_A = 0.0
        for a in self.attrs[attr]:
            # 속성이 a인 row들을 뽑음 (ex: attr(income)이 a (high, medium, low)인 애들을 각각 뽑음)
            tmp_df = df[df[attr] == a]  
            # 각각에 대해 entropy를 계산하고 가중치를 곱해줌
            info_A += (len(tmp_df) / len(df)) * self.entropy(self.probability(tmp_df))
        
        gain = info_D - info_A
        return gain

    '''
    Split information 값을 계산한다.
    '''
    def split_info(self, df, attr):
        splitInfo = 0.0
        for a in self.attrs[attr]:
            # 속성이 a인 row들을 뽑음
            tmp_df = df[df[attr] == a]
            p = len(tmp_df) / len(df)
            splitInfo += (-1.0) * p * (math.log(p + 1e-15,2))
        
        return splitInfo

    '''
    Information gain과 split information을 통해 gain ratio를 구한다.
    '''
    def gain_ratio(self, df, attr):
        gain = self.information_gain(df, attr)
        splitInfo = self.split_info(df, attr)

        return gain / splitInfo

    '''
    Decision tree를 생성한다.
    '''
    def create_dt(self, df):
        # Data frame이 분류가 완벽하여 label이 한가지만 존재하는 경우
        # Label을 정한다.
        if df[self.label].nunique() == 1:
            label = df[self.label].unique()[0]
            return Node(None, True, label)

        # 분류 방법으로 gain ratio를 이용한다.
        result_dict = {attr: self.gain_ratio(df, attr) for attr in df.columns if attr!=self.label}
        selected_attr = sorted(result_dict.items(), key=lambda x: x[1], reverse = True)[0][0]
        
        # 노드를 생성하고 선택된 속성을 기준으로 split한다.
        node = Node(selected_attr, False, None)

        a = self.attrs[node.attr]
        for i in range(len(a)):
            tmp_df = df[df[node.attr].values == a[i]]
            tmp_df = tmp_df.drop(columns=node.attr,axis=1)
            if (len(tmp_df) > 0):
                node.child[a[i]] = self.create_dt(tmp_df)
                
        return node
        
    '''
    Data가 주어진 경우 해당 data의 leaf node를 찾는다.
    '''
    def find_leaf(self, data):
        node = self.root
        a = data[node.attr]
        while not node.isleaf:
            for (child, next_node) in node.child.items():
                if a in child:
                    break
            node = next_node
            if node.attr:
                a = data[node.attr]
        
        return node.label

    '''
    Data frame을 받아서 label을 분류해준다.
    '''
    def classify(self, df):
        label = [self.find_leaf(df.loc[i]) for i in range(len(df))]
        df[self.label] = label
        return df


def main():
    train_file = sys.argv[1]
    test_file = sys.argv[2]
    result_file = sys.argv[3]

    train_df = pd.read_csv(train_file, sep='\t')
    test_df = pd.read_csv(test_file, sep='\t')

    dt = DecisionTree(train_df)

    df = dt.classify(test_df)
    df.to_csv(result_file, index=False, sep='\t')
    

if __name__ == "__main__":
    main()
