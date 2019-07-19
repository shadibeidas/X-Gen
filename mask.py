#bleh.... removing backrounds and crap

count = 100 #count to base background average off

edge = 50 #number of pixels considered bad on each edge

def Mask(inputfile):
    #removing the spots to even out the image
    print "Masking image:"
    print "-removing spots in image",
    for x in range(len(inputfile)):
        if x % 300 == 0:
            print ".",
        for y in range(len(inputfile[x])):
            if inputfile[x][y] > (3 * count):
                inputfile[x][y] = count
    #making the mask
    print "\n-making mask",
    for x in range(len(inputfile)):
        if x % 300 == 0:
            print ".",
        for y in range(len(inputfile[x])):
            if inputfile[x][y] < (0.35 * count):#make initial mask by finding spots with lowest values
                inputfile[x][y] = 65535 #use max value for bad area
            elif x <edge or x > (len(inputfile)-edge):#remove spots on top and bottom edges
                inputfile[x][y] = 65535 #use max value for bad area
            elif y <edge or y > (len(inputfile[x])-edge):#remove spots on side edges
                inputfile[x][y] = 65535 #use max value for bad area
            else:
                inputfile[x][y] = 0 #use 0 for a good spot
    #smoothing the mask
    print "\n-smoothing mask",
    for x in range(len(inputfile)):
        if x % 300 == 0:
            print ".",
        for y in range(len(inputfile[x])):
            if inputfile[x][y] == 0:#masks unmasked points in need of masking
                n=0
                for i in range(x-1, x+1):
                    for j in range(y-1,y+1):
                        if inputfile[i][j] == 65535:
                            n+=1
                if n > 4:
                    inputfile[x][y] = 65535
            if inputfile[x][y] == 65535:#finds and removes lone masked points
                n=0
                for i in range(x-1, x+1):
                    for j in range(y-1,y+1):
                        if inputfile[i][j] == 0:
                            n+=1  
                if n > 3:
                    inputfile[x][y] = 0
    print "\n---returning mask"
    return inputfile