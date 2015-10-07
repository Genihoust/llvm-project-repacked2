import curses, curses.panel
import sys
import time 

class Point(object):
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __repr__(self):
        return str(self)

    def __str__(self):
        return "(x=%u, y=%u)" % (self.x, self.y)
    
    def is_valid_coordinate(self):
        return self.x >= 0 and self.y >= 0
        
class Size(object):
    def __init__(self, w, h):
        self.w = w
        self.h = h

    def __repr__(self):
        return str(self)

    def __str__(self):
        return "(w=%u, h=%u)" % (self.w, self.h)

class Rect(object):
    def __init__(self, x=0, y=0, w=0, h=0):
        self.origin = Point(x, y)
        self.size = Size(w, h)
        
    def __repr__(self):
        return str(self)

    def __str__(self):
        return "{ %s, %s }" % (str(self.origin), str(self.size))

    def get_min_x(self):
        return self.origin.x

    def get_max_x(self):
        return self.origin.x + self.size.w

    def get_min_y(self):
        return self.origin.y

    def get_max_y(self):
        return self.origin.y + self.size.h
    
    def contains_point(self, pt):
        if pt.x < self.get_max_x():
            if pt.y < self.get_max_y():
                if pt.x >= self.get_min_y():
                    return pt.y >= self.get_min_y()
        return False

class Window(object):
    def __init__(self, window, delegate = None, can_become_first_responder = True):
        self.window = window
        self.parent = None
        self.delegate = delegate
        self.children = list()
        self.first_responders = list()
        self.can_become_first_responder = can_become_first_responder
        self.key_actions = dict()
    
    def add_child(self, window):
        self.children.append(window)
        window.parent = self

    def add_key_action(self, arg, callback, decription):
        if isinstance(arg, list):
            for key in arg:
                self.add_key_action(key, callback, description)
        else:
            if isinstance(arg, ( int, long )):
                key_action_dict = { 'key'         : arg, 
                                    'callback'    : callback,
                                    'description' : decription }
                self.key_actions[arg] = key_action_dict
            elif isinstance(arg, basestring):       
                key_integer = ord(arg)
                key_action_dict = { 'key'         : key_integer, 
                                    'callback'    : callback,
                                    'description' : decription }
                self.key_actions[key_integer] = key_action_dict
            else:
                raise ValueError
       
    def remove_child(self, window):
        self.children.remove(window)
                                
    def get_first_responder(self):
        if len(self.first_responders):
            return self.first_responders[-1]
        else:
            return None

    def set_first_responder(self, window):
        if window.can_become_first_responder:
            if callable(getattr(window, "hidden", None)) and window.hidden():
                return False
            if not window in self.children:
                self.add_child(window)
            # See if we have a current first responder, and if we do, let it know that
            # it will be resigning as first responder  
            first_responder = self.get_first_responder()
            if first_responder:
                first_responder.relinquish_first_responder()
            # Now set the first responder to "window"
            if len(self.first_responders) == 0:
                self.first_responders.append(window)
            else:
                self.first_responders[-1] = window
            return True
        else:
            return False
    
    def push_first_responder(self, window):
        # Only push the window as the new first responder if the window isn't already the first responder
        if window != self.get_first_responder():
            self.first_responders.append(window)
        
    def pop_first_responder(self, window):                                                               
        # Only pop the window from the first responder list if it is the first responder
        if window == self.get_first_responder():
            old_first_responder = self.first_responders.pop()
            old_first_responder.relinquish_first_responder()
            return True
        else:
            return False
        
    def relinquish_first_responder(self):
        '''Override if there is something that you need to do when you lose first responder status.'''
        pass                                                                                       
        
    # def resign_first_responder(self, remove_from_parent, new_first_responder):   
    #     success = False
    #     if self.parent:
    #         if self.is_first_responder():   
    #             self.relinquish_first_responder()
    #             if len(self.parent.first_responder):
    #             self.parent.first_responder = None
    #             success = True
    #         if remove_from_parent:
    #             self.parent.remove_child(self)
    #         if new_first_responder:
    #             self.parent.set_first_responder(new_first_responder)
    #         else:
    #             self.parent.select_next_first_responder()
    #     return success

    def is_first_responder(self):
        if self.parent:
            return self.parent.get_first_responder() == self
        else:
            return False

    def is_in_first_responder_chain(self):
        if self.parent:
            return self in self.parent.first_responders
        else:
            return False

    def select_next_first_responder(self):
        if len(self.first_responders) > 1:
            self.pop_first_responder(self.first_responders[-1])
        else:
            num_children = len(self.children)
            if num_children == 1:
                return self.set_first_responder(self.children[0])
            for (i,window) in enumerate(self.children):
                if window.is_first_responder():
                    break
            if i < num_children:
                for i in range(i+1,num_children):
                    if self.set_first_responder(self.children[i]):
                        return True
                for i in range(0, i):
                    if self.set_first_responder(self.children[i]):
                        return True
            
    def point_in_window(self, pt):
        size = self.get_size()
        return pt.x >= 0 and pt.x < size.w and pt.y >= 0 and pt.y < size.h
    
    def addch(self, pt, c):
        try:
            self.window.addch(pt.y, pt.x, c)
        except:
            pass

    def addstr(self, pt, str):
        try:
            self.window.addstr(pt.y, pt.x, str)
        except:
            pass

    def addnstr(self, pt, str, n):
        try:
            self.window.addnstr(pt.y, pt.x, str, n)
        except:
            pass

    def attron(self, attr):
        return self.window.attron (attr)

    def attroff(self, attr):
        return self.window.attroff (attr)

    def box(self, vertch=0, horch=0):
        if vertch == 0:
            vertch = curses.ACS_VLINE
        if horch == 0: 
            horch = curses.ACS_HLINE
        self.window.box(vertch, horch)

    def get_contained_rect(self, top_inset=0, bottom_inset=0, left_inset=0, right_inset=0, height=-1, width=-1):
        '''Get a rectangle based on the top "height" lines of this window'''
        rect = self.get_frame()
        x = rect.origin.x + left_inset
        y = rect.origin.y + top_inset
        if height == -1:
            h = rect.size.h - (top_inset + bottom_inset)
        else:
            h = height
        if width == -1:
            w = rect.size.w - (left_inset + right_inset)
        else:
            w = width
        return Rect (x = x, y = y, w = w, h = h)

    def erase(self):
        self.window.erase()

    def get_frame(self):
        position = self.get_position()
        size = self.get_size()
        return Rect(x=position.x, y=position.y, w=size.w, h=size.h)

    def get_position(self):
        (y, x) = self.window.getbegyx()
        return Point(x, y)

    def get_size(self):
        (y, x) = self.window.getmaxyx()
        return Size(w=x, h=y)

    def refresh(self):
        self.update()
        curses.panel.update_panels()
        return self.window.refresh()
        
    def resize(self, size):
        return self.window.resize(size.h, size.w)
    
    def timeout(self, timeout_msec):
        return self.window.timeout(timeout_msec)

    def handle_key(self, key, check_parent=True):
        '''Handle a key press in this window.'''
        
        # First try the first responder if this window has one, but don't allow
        # it to check with its parent (False second parameter) so we don't recurse
        # and get a stack overflow
        for first_responder in reversed(self.first_responders):
            if first_responder.handle_key(key, False):
                return True       

        # Check our key map to see if we have any actions. Actions don't take
        # any arguments, they must be callable
        if key in self.key_actions:
            key_action = self.key_actions[key]
            key_action['callback']()
            return True
        # Check if there is a wildcard key for any key
        if -1 in self.key_actions:
            key_action = self.key_actions[-1]
            key_action['callback']()
            return True
        # Check if the window delegate wants to handle this key press
        if self.delegate:      
            if callable(getattr(self.delegate, "handle_key", None)):
                if self.delegate.handle_key(self, key):
                    return True
            if self.delegate(self, key):
                return True
        # Check if we have a parent window and if so, let the parent 
        # window handle the key press
        if check_parent and self.parent:
            return self.parent.handle_key(key, True)
        else:
            return False # Key not handled

    def update(self):
        for child in self.children:
            child.update()

    def key_event_loop(self, timeout_msec=-1, n=sys.maxint):
        '''Run an event loop to receive key presses and pass them along to the
           responder chain.
           
           timeout_msec is the timeout it milliseconds. If the value is -1, an
           infinite wait will be used. It the value is zero, a non-blocking mode
           will be used, and if greater than zero it will wait for a key press
           for timeout_msec milliseconds.
           
           n is the number of times to go through the event loop before exiting'''
        self.timeout(timeout_msec)
        while n > 0:
            c = self.window.getch()
            if c != -1:
                try:
                    self.handle_key(c)
                except:
                    break
            n -= 1

class Panel(Window):
    def __init__(self, frame, delegate = None, can_become_first_responder = True):
        window = curses.newwin(frame.size.h,frame.size.w, frame.origin.y, frame.origin.x)
        super(Panel, self).__init__(window, delegate, can_become_first_responder)
        self.panel = curses.panel.new_panel(window)

    def hide(self):
        return self.panel.hide()

    def hidden(self):
        return self.panel.hidden()

    def show(self):
        return self.panel.show()

    def top(self):
        return self.panel.top()
    
    def set_position(self, pt):
        self.panel.move(pt.y, pt.x)
    
    def slide_position(self, pt):
        new_position = self.get_position()
        new_position.x = new_position.x + pt.x
        new_position.y = new_position.y + pt.y
        self.set_position(new_position)

class BoxedPanel(Panel):
    def __init__(self, frame, title, delegate = None, can_become_first_responder = True):
        super(BoxedPanel, self).__init__(frame, delegate, can_become_first_responder)
        self.title = title
        self.lines = list()
        self.first_visible_idx = 0
        self.selected_idx = -1
        self.add_key_action(curses.KEY_UP,   self.select_prev, "Select the previous item")
        self.add_key_action(curses.KEY_DOWN, self.select_next, "Select the next item")
        self.add_key_action(curses.KEY_HOME, self.scroll_begin, "Go to the beginning of the list")
        self.add_key_action(curses.KEY_END,  self.scroll_end,   "Go to the end of the list")
        self.add_key_action(curses.KEY_PPAGE, self.scroll_page_backward, "Scroll to previous page")
        self.add_key_action(curses.KEY_NPAGE, self.scroll_page_forward, "Scroll to next forward")
        self.update()

    def clear(self, update=True):
        self.lines = list()
        self.first_visible_idx = 0
        self.selected_idx = -1
        if update:
            self.update()
    
    def get_usable_width(self):
        '''Valid usable width is 0 to (width - 3) since the left and right lines display the box around 
           this frame and we skip a leading space'''
        w = self.get_size().w
        if w > 3:
            return w-3
        else:
            return 0
    
    def get_usable_height(self):
        '''Valid line indexes are 0 to (height - 2) since the top and bottom lines display the box around this frame.'''
        h = self.get_size().h
        if h > 2:
            return h-2
        else:
            return 0

    def get_point_for_line(self, global_line_idx):
        '''Returns the point to use when displaying a line whose index is "line_idx"'''
        line_idx = global_line_idx - self.first_visible_idx
        num_lines = self.get_usable_height()
        if line_idx < num_lines:
            return Point(x=2, y=1+line_idx)
        else:
            return Point(x=-1, y=-1) # return an invalid coordinate if the line index isn't valid
        
    def set_title (self, title, update=True):
        self.title = title
        if update:
            self.update()

    def scroll_begin (self):
        self.first_visible_idx = 0
        if len(self.lines) > 0:
            self.selected_idx = 0
        else:
            self.selected_idx = -1
        self.update()

    def scroll_end (self):
        max_visible_lines = self.get_usable_height()
        num_lines = len(self.lines)
        if num_lines > max_visible_lines:
            self.first_visible_idx = num_lines - max_visible_lines
        else:
            self.first_visible_idx = 0
        self.selected_idx = num_lines-1
        self.update()
    
    def scroll_page_backward(self):
        num_lines = len(self.lines) 
        max_visible_lines = self.get_usable_height()        
        new_index = self.first_visible_idx - max_visible_lines
        if new_index < 0:
            self.first_visible_idx = 0
        else:
            self.first_visible_idx = new_index
        self.refresh()
 
    def scroll_page_forward(self):
        max_visible_lines = self.get_usable_height()        
        self.first_visible_idx += max_visible_lines
        self._adjust_first_visible_line()
        self.refresh()

    def select_next (self):
        self.selected_idx += 1
        if self.selected_idx >= len(self.lines):
            self.selected_idx = len(self.lines) - 1
        self.refresh()
        
    def select_prev (self):
        self.selected_idx -= 1
        if self.selected_idx < 0:
            if len(self.lines) > 0:
                self.selected_idx = 0
            else:
                self.selected_idx = -1
        self.refresh()

    def get_selected_idx(self):
        return self.selected_idx
    
    def _adjust_first_visible_line(self):
        num_lines = len(self.lines)
        max_visible_lines = self.get_usable_height()
        if (self.first_visible_idx >= num_lines) or (num_lines - self.first_visible_idx) > max_visible_lines:
            self.first_visible_idx = num_lines - max_visible_lines
        
    def append_line(self, s, update=True):
        self.lines.append(s)
        self._adjust_first_visible_line()
        if update:
            self.update()

    def set_line(self, line_idx, s, update=True):
        '''Sets a line "line_idx" within the boxed panel to be "s"'''
        if line_idx < 0:
            return
        while line_idx >= len(self.lines):
            self.lines.append('')
        self.lines[line_idx] = s
        self._adjust_first_visible_line()
        if update:
            self.update()
    
    def update(self):
        self.erase()
        is_in_first_responder_chain = self.is_in_first_responder_chain()
        if is_in_first_responder_chain:
            self.attron (curses.A_REVERSE)
        self.box()
        if is_in_first_responder_chain:
            self.attroff (curses.A_REVERSE)
        if self.title:
            self.addstr(Point(x=2, y=0), ' ' + self.title + ' ')
        max_width = self.get_usable_width()
        for line_idx in range(self.first_visible_idx, len(self.lines)):
            pt = self.get_point_for_line(line_idx)
            if pt.is_valid_coordinate():
                is_selected = line_idx == self.selected_idx
                if is_selected:
                    self.attron (curses.A_REVERSE)
                self.addnstr(pt, self.lines[line_idx], max_width)
                if is_selected:
                    self.attroff (curses.A_REVERSE)
            else:
                return

class Item(object):
    def __init__(self, title, action):
        self.title = title
        self.action = action
    
class Menu(BoxedPanel):
    def __init__(self, title, items):
        max_title_width = 0
        for item in items:
            if max_title_width < len(item.title):
                max_title_width = len(item.title)
        frame = Rect(x=0, y=0, w=max_title_width+4, h=len(items)+2)
        super(Menu, self).__init__(frame, title=None, delegate=None, can_become_first_responder=True)
        self.selected_idx = 0
        self.title = title
        self.items = items
        for (item_idx, item) in enumerate(items):
            self.set_line(item_idx, item.title)
        self.hide()
    
    def update(self):
        super(Menu, self).update()

    def relinquish_first_responder(self):
        if not self.hidden():
            self.hide()                            
    
    def perform_action(self):           
        selected_idx = self.get_selected_idx()
        if selected_idx < len(self.items):
            action = self.items[selected_idx].action
            if action:
                action()
        
class MenuBar(Panel):
    def __init__(self, frame):
        super(MenuBar, self).__init__(frame, can_become_first_responder=True)
        self.menus = list()
        self.selected_menu_idx = -1
        self.add_key_action(curses.KEY_LEFT,  self.select_prev, "Select the previous menu")
        self.add_key_action(curses.KEY_RIGHT, self.select_next, "Select the next menu")
        self.add_key_action(curses.KEY_DOWN,  lambda: self.select(0), "Select the first menu")
        self.add_key_action(27, self.relinquish_first_responder, "Hide current menu")
        self.add_key_action(curses.KEY_ENTER, self.perform_action, "Select the next menu item")
        self.add_key_action(10, self.perform_action, "Select the next menu item")

    def insert_menu(self, menu, index=sys.maxint):
        if index >= len(self.menus):
            self.menus.append(menu)
        else:
            self.menus.insert(index, menu)
        pt = self.get_position()
        for menu in self.menus:
            menu.set_position(pt)
            pt.x += len(menu.title) + 5
    
    def perform_action(self):   
        '''If no menu is visible, show the first menu. If a menu is visible, perform the action
           associated with the selected menu item in the menu'''
        menu_visible = False
        for menu in self.menus:
            if not menu.hidden():
                menu_visible = True
                break
        if menu_visible:
            menu.perform_action()
            self.selected_menu_idx = -1
            self._selected_menu_changed()
        else:
            self.select(0)
        
    def relinquish_first_responder(self):
        if self.selected_menu_idx >= 0:
            self.selected_menu_idx = -1
            self._selected_menu_changed()
 
    def _selected_menu_changed(self):
        for (menu_idx, menu) in enumerate(self.menus):
            is_hidden = menu.hidden()
            if menu_idx != self.selected_menu_idx:  
                if not is_hidden:
                    if self.parent.pop_first_responder(menu) == False:
                        menu.hide()
        for (menu_idx, menu) in enumerate(self.menus):
            is_hidden = menu.hidden()
            if menu_idx == self.selected_menu_idx:  
                if is_hidden:
                    menu.show()
                    self.parent.push_first_responder(menu)
                menu.top()
        self.parent.refresh()

    def select(self, index):
        if index < len(self.menus):
            self.selected_menu_idx = index
            self._selected_menu_changed()
            
    def select_next (self):
        num_menus = len(self.menus)
        if self.selected_menu_idx == -1:
            if num_menus > 0:
                self.selected_menu_idx = 0
                self._selected_menu_changed()
        else:
            if self.selected_menu_idx + 1 < num_menus:
                self.selected_menu_idx += 1
            else:
                self.selected_menu_idx = -1
            self._selected_menu_changed()

    def select_prev (self):
        num_menus = len(self.menus)
        if self.selected_menu_idx == -1:
            if num_menus > 0:
                self.selected_menu_idx = num_menus - 1
                self._selected_menu_changed()
        else:
            if self.selected_menu_idx - 1 >= 0:
                self.selected_menu_idx -= 1
            else:
                self.selected_menu_idx = -1
            self._selected_menu_changed()

    def update(self):
        self.erase()
        is_in_first_responder_chain = self.is_in_first_responder_chain()
        if is_in_first_responder_chain:
            self.attron (curses.A_REVERSE)
        pt = Point(x=0, y=0)
        for menu in self.menus:          
            self.addstr(pt, '|  ' + menu.title + '  ')
            pt.x += len(menu.title) + 5
        self.addstr(pt, '|')      
        width = self.get_size().w
        while pt.x < width:
            self.addch(pt, ' ')
            pt.x += 1  
        if is_in_first_responder_chain:
            self.attroff (curses.A_REVERSE)

        for menu in self.menus:
            menu.update()
        
        
class StatusPanel(Panel):
    def __init__(self, frame):
        super(StatusPanel, self).__init__(frame, delegate=None, can_become_first_responder=False)
        self.status_items = list()
        self.status_dicts = dict()
        self.next_status_x = 1
    
    def add_status_item(self, name, title, format, width, value, update=True):
        status_item_dict = { 'name': name,
                             'title' : title,
                             'width' : width,
                             'format' : format,
                             'value' : value,
                             'x' : self.next_status_x }
        index = len(self.status_items)
        self.status_items.append(status_item_dict)
        self.status_dicts[name] = index
        self.next_status_x += width + 2;
        if update:
            self.update()

    def increment_status(self, name, update=True):
        if name in self.status_dicts:
            status_item_idx = self.status_dicts[name]
            status_item_dict = self.status_items[status_item_idx]
            status_item_dict['value'] = status_item_dict['value'] + 1
        if update:
            self.update()
    
    def update_status(self, name, value, update=True):
        if name in self.status_dicts:
            status_item_idx = self.status_dicts[name]
            status_item_dict = self.status_items[status_item_idx]
            status_item_dict['value'] = status_item_dict['format'] % (value)
        if update:
            self.update()
    def update(self):
        self.erase();
        for status_item_dict in self.status_items:
            self.addnstr(Point(x=status_item_dict['x'], y=0), '%s: %s' % (status_item_dict['title'], status_item_dict['value']), status_item_dict['width'])

stdscr = None

def intialize_curses():
    global stdscr
    stdscr = curses.initscr()
    curses.noecho()
    curses.cbreak()
    stdscr.keypad(1)
    try:
        curses.start_color()
    except:
        pass
    return Window(stdscr)

def terminate_curses():
    global stdscr
    if stdscr:
        stdscr.keypad(0)
    curses.echo()
    curses.nocbreak()
    curses.endwin()

