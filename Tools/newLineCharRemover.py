data_ = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    <button style="width: 100px; height: 100px; margin: auto;">Run</button>
</body>
</html>
"""


data_ = data_.replace("\n", "")
data_ = data_.replace('"', '\\"')
new_data = ""
new_data = new_data.join(data_.split("  "))
print(new_data);
