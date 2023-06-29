import requests

t_data = "VGA,3,4,2,-2,1,2,"
data_ = requests.post("http://192.168.43.46/change_settings", t_data)
print(data_.text)
