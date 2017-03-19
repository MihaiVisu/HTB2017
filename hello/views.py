from django.shortcuts import render
from django.http import HttpResponse

from .models import Greeting

# Create your views here.
def index(request):
    # return HttpResponse('Hello from Python!')
    return render(request, 'index.html')


def db(request):

    greeting = Greeting()
    greeting.save()

    greetings = Greeting.objects.all()

    return render(request, 'db.html', {'greetings': greetings})


def response(request):
    sid = SentimentIntensityAnalyzer()

    response = urllib2.urlopen("https://arduino-sticky-notes.herokuapp.com/twitter/realdonaldtrump").read()

    ss = sid.polarity_scores(response)
    mx = -1
    res = "neu"
    for k in ss:
        if ss[k] > mx and k != "compound":
            mx = ss[k]
            res = k

    return response + "\n" + res
