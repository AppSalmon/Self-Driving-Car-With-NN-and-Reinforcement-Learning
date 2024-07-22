import numpy as np
from sklearn.neural_network import MLPClassifier
import joblib

# Training data: [distanceF, distanceR, distanceL]
# 0 = backward, 1 = forward, 2 = left, 3 = right
X = np.array([
    [100, 50, 70], [30, 100, 70], [30, 50, 100],
    [100, 20, 50], [20, 100, 50], [20, 50, 100],
    [100, 40, 60], [40, 100, 60], [40, 60, 100]
])
y = np.array([0, 1, 2, 0, 1, 2, 0, 1, 2])

# Hàm tạo dữ liệu mới bằng cách thay đổi ngẫu nhiên
def generate_data(X, y, num_samples=100):
    new_X = []
    new_y = []
    
    for _ in range(num_samples):
        for xi, yi in zip(X, y):
            # Tạo nhiễu ngẫu nhiên trong khoảng -10 đến 10 cho mỗi giá trị khoảng cách
            noise = np.random.uniform(-10, 10, size=xi.shape)
            new_xi = xi + noise
            new_X.append(new_xi)
            new_y.append(yi)
    
    return np.array(new_X), np.array(new_y)

# Tạo thêm dữ liệu huấn luyện
X_new, y_new = generate_data(X, y, num_samples=10)

# Kết hợp dữ liệu cũ và mới
X_combined = np.vstack((X, X_new))
y_combined = np.hstack((y, y_new))

print("Dữ liệu đầu vào mới:")
print(X_combined)
print("Nhãn mới:")
print(y_combined)