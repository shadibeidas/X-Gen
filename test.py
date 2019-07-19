import wx
import os
import matplotlib as mpl
mpl.use('WXAgg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as Canvas

import marccd #file for reading marccd images
import mask #file for masking image

ID_FILE_ABOUT = wx.NewId()

inputfile = 'VHbA4_1.0001'

class Plot(wx.Panel):
    def __init__(self, parent, id = -1, dpi = None, **kwargs):
        wx.Panel.__init__(self, parent, id=id, **kwargs)
        imagemask = mask.Mask(marccd.image(inputfile))
        plt.imshow(marccd.image(inputfile))#switch for mask
        self.figure = plt.gcf()
        self.figure.set_size_inches(4, 4)
        self.canvas = Canvas(self, -1, self.figure)
        ###closeButton = wx.Button(self, -1, "Close", pos=(20,20))
        
        ###self.Bind(wx.EVT_BUTTON, self.closeTab, closeButton)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.canvas,1,wx.EXPAND)
        self.SetSizer(sizer)
        
    '''def closeTab(self, e):
        DeletePage(self)'''

class JBC(wx.Frame):
    def __init__(self, parent, id, title):
        wx.Frame.__init__(self, parent, id, title, size=(800,600))
        ###self.SetBackgroundColour(wx.Colour(150, 100, 200))
        self.CreateStatusBar() # A StatusBar in the bottom of the window
        
        self.SetStatusText("Weeeeee, status bars!")
        
        # Setting up the menu.
        filemenu= wx.Menu()

        # wx.ID_ABOUT and wx.ID_EXIT are standard IDs provided by wxWidgets.
        filemenu.Append(wx.ID_OPEN, "&Open", "Open a file to read")
        filemenu.AppendSeparator()
        filemenu.Append(ID_FILE_ABOUT, "&About"," Information about this program")
        filemenu.AppendSeparator()
        filemenu.Append(wx.ID_EXIT,"E&xit"," Terminate the program")

        # Creating the menubar. Broken
        menuBar = wx.MenuBar()
        menuBar.Append(filemenu,"&File") # Adding the "filemenu" to the MenuBar
        self.SetMenuBar(menuBar)  # Adding the MenuBar to the Frame content.
        #self.Show(True)
        
        #Creating image area
        self.nbG = wx.Notebook(self, -1, style=0, size=(500,500), pos=(0,0))
        self.gSheet1 = self.add(inputfile).gca()
        #moved to run
        
        
        #button to test displaying the info after window is loaded
        runButton = wx.Button(self, wx.NewId(), "Run", pos=(0,
        self.nbG.Position.y+500))
        
        #button to mask image
        maskButton = wx.Button(self, wx.NewId(), "Mask", pos=(100,
        self.nbG.Position.y+500))
        
        #button to select a new file
        openButton = wx.Button(self, wx.NewId(), "Open", pos=(200,
        self.nbG.Position.y+500))
        
        #button to kill window while i figure out how to actually close it
        killButton = wx.Button(self, wx.NewId(), "Kill", pos=(300,
        self.nbG.Position.y+500))
        
        #some information on the image displayed
        headNameText = wx.StaticText(self, wx.NewId(), "Header name: " + 
        marccd_dict["header_name"], pos=(self.nbG.Position.x+500, 0))
        totalCountsText = wx.StaticText(self, wx.NewId(), "Header size: " + 
        str(marccd_dict["header_size"]), pos=(self.nbG.Position.x+500, 20))
        maxText = wx.StaticText(self, wx.NewId(), "Maximum: " + 
        str(marccd_dict["max"]), pos=(self.nbG.Position.x+500, 40))

        # Set events. Not really working for the menus
        self.Bind(wx.EVT_MENU, self.OnOpen)
        wx.EVT_MENU(self, ID_FILE_ABOUT, self.OnAbout)
        self.Bind(wx.EVT_MENU, self.OnExit)#, menuExit)
        self.Bind(wx.EVT_BUTTON, self.OnKill, killButton)
        self.Bind(wx.EVT_BUTTON, self.OnRun, runButton)
        self.Bind(wx.EVT_BUTTON, self.OnOpen, openButton)
        self.Bind(wx.EVT_BUTTON, self.OnMask, maskButton)
        # self.Show(True)
        
    def OnOpen(self,e):
        """ Open a file"""
        self.dirname = ''
        dlg = wx.FileDialog(self, "Choose a file", self.dirname, "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            self.filename = dlg.GetFilename()
            self.dirname = dlg.GetDirectory()
            globals()["inputfile"] = self.filename
            print inputfile
        dlg.Destroy()
        
    def OnRun(self, e):
        print inputfile
        self.gSheet1 = self.add(inputfile).gca()
        '''plt.imshow(marccd.image(inputfile))
        Plot.figure = plt.gcf()
        Plot.Show()'''
        
    def OnMask(self, e):
        print "masking " + inputfile
        print "this still does not work"

    def OnKill(self, e):
        #self.Close(True)
        exit(0)
        
    def OnAbout(self,e):
        # A message dialog box with an OK button. wx.OK is a standard ID in wxWidgets.
        dlg = wx.MessageDialog( self, "A test program", "About Test", wx.OK)
        dlg.ShowModal() # Show it
        dlg.Destroy() # finally destroy it when finished.

    def OnExit(self,e):
        #self.Close(True)  # Close the frame.
        exit(0)

    def add(self,name="plot"):
       page = Plot(self.nbG)
       self.nbG.AddPage(page,name)
       return page.figure

    def Update(self):
        self.gSheet1.clear()
        plt.draw()
        print "Tried to redraw"

if __name__ == '__main__':
    print "Starting Test Program"
    app = wx.App()
    print "Calling for header information"
    marccd_dict = marccd.header(inputfile)#assuming this is a fair place for it
    print "Preparing window"
    frame=JBC(None, -1, "Crystallography Test Program")
    print "Launching window"
    frame.Show()
    print "Application running"
    app.MainLoop()

