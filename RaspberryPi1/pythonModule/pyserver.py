from flask import Flask, request, jsonify
from PIL import Image
import torch
import torch.nn as nn
import torchvision
from PIL import Image
import torchvision
import torchvision.transforms as transforms
import Wongi.pysocket as pysocket
import warnings
warnings.filterwarnings("ignore")

app = Flask(__name__)
UPLOAD_FOLDER = 'uploads'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"

valid_transforms = transforms.Compose([
    transforms.Resize((448, 448)),
    transforms.CenterCrop(224),
    transforms.ToTensor(),
    transforms.Normalize(mean=[0.485, 0.456, 0.406],
                         std=[0.229, 0.224, 0.225])
    ])

@app.route('/upload', methods=['POST'])
def upload_image():
    if 'image' not in request.files:
        return jsonify({'error': 'No image provided'})
    else:
        image = request.files['image']
        image.save('img.jpg')
        model = torchvision.models.resnet50(pretrained = True)
        model.fc = nn.Linear(model.fc.in_features, 4)
        model.eval()
        model.load_state_dict(torch.load("/Users/wongipark/Desktop/SP/TeamProject/model_20.pt", map_location="cpu"))
        
        img = "/Users/wongipark/Desktop/SP/TeamProject/img.jpg"
        img1 = Image.open(img)
        temp_img = valid_transforms(img1)
        temp_img = temp_img.unsqueeze(0)

        a = model(temp_img)
        w = torch.argmax(a)
        
        if int(w) != 1:
            message = "TWO"
            pysocket.s.send(message.encode('utf-8'))        
        else:
            message = "one"
            pysocket.s.send(message.encode('utf-8'))        
            
        return jsonify({'number': int(1)})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=9999, debug = True)