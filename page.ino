Page setValue(Page *P, int value) {
  P->currentValue = value;
  return *P;
};

Page setState(Page *P, int state) {
  P->state = state;
  return *P;
}

Page* getPageByName(Page *pages, const String &name, int pagesLength) {
  for(int i = 0; i < pagesLength; ++i) {
    if(pages[i].name == name) {
      return &pages[i];
    }
  }
  return nullptr;
}
