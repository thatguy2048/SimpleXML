#include "XML.h"
#include <sstream>

namespace XML{
	
//-------------------------Escape-Characters---------------------------------//
	const std::string escapeCharacters = "\"'&><";
	const std::string XML::escapeStrings[] = {"&quot;","&apos;","&amp;","&gt;","&lt;"};
	const unsigned int NumberOfEscapeCharacters = escapeCharacters.length();
	
	inline bool ContainsAnEscapeCharacter(const std::string& str){
		return (str.find_first_of(escapeCharacters) != std::string::npos);
	}
	
	std::string EscapeCharacterToString(char c){
		for(unsigned int i = 0; i < NumberOfEscapeCharacters; ++i){
			if(escapeCharacters[i] == c)	return escapeStrings[i];
		}
		return std::string(1,c);
	}
	
	std::ostream& WriteEscapeCharacterToStream(std::ostream& os, char c){
		for(unsigned int i = 0; i < NumberOfEscapeCharacters; ++i){
			if(escapeCharacters[i] == c){
				os << escapeStrings[i];
				break;
			}
		}
		return os;
	}
	
	char EscapeStringToCharacter(const std::string& str){
		for(unsigned int i = 0; i < NumberOfEscapeCharacters; ++i){
			if(escapeStrings[i] == str)	return escapeCharacters[i];
		}
		return '\0';
	}
	
	bool StrComp(const char * str1, const char * str2, unsigned int strlen){
		while(strlen > 0){
			strlen--;
			if(str1[strlen] != str2[strlen]){
				return false;
			}
		}
		return true;
	}
	
	std::size_t IsEscapeString(const char * str){
		for(unsigned int i = 0; i < NumberOfEscapeCharacters; ++i){
			if(StrComp(str,escapeStrings[i].begin(), escapeStrings[i].length())){
				return i;
			}
		}
		return std::string::npos;
	}
	
	std::string EscapeString(const std::string& str){
		std::stringstream output;
		std::size_t pfnd = 0;
        std::size_t fnd = str.find_first_of(escapeCharacters);
		while(fnd != std::string::npos){
			output.write(&str[pfnd],fnd-pfnd);
			WriteEscapeCharacterToStream(output, str[fnd]);
			pfnd = fnd+1;
			fnd = str.find_first_of(escapeCharacters,pfnd);
		}
		output.write(&str[pfnd],str.length()-pfnd);
		return output.str();
	}
	
	std::string DeEscapeString(const std::string& str){
        std::stringstream output;
		std::size_t pfnd = 0;
		std::size_t fnd = str.find('&');
		while(fnd != std::string::npos){
            std::size_t tmp_fnd = IsEscapeString(&str[fnd]);
			if(tmp_fnd != std::string::npos){
				output.write(&str[pfnd],fnd-pfnd);
				output << escapeCharacters[tmp_fnd];
				pfnd = fnd+escapeStrings[tmp_fnd].length();
				fnd = str.find('&',pfnd);
			}else{
				fnd = str.find('&',fnd+1);
			}
		}
		output.write(&str[pfnd],str.length()-pfnd);
        return output.str();
	}

//-------------------------------------PUBLIC------------------------------------------//

	XML::Object* XML::Copy(const Object* other){
		XML::Object* output;
		if(other == NULL){
			output = NULL;
		}else{			
			switch(other->getType()){
				case XML_STRING:
					output = ((XML::String*)other)->copy();
					break;
				case XML_TAG:
					output = ((XML::Tag*)other)->copy();
					break;
				default:
					output = other->copy();
					break;
			};
		}
		return output;
	}
	
//-----------------------------------OBJECT-----------------------------------------------//

	Object::Object():parent(NULL), type(XML_OBJECT){}
	Object::Object(const std::string& Name, Tag * Parent):name(Name), parent(Parent), type(XML_OBJECT){}
	Object::Object(const Object& other):name(other.name), parent(other.parent), type(other.type){}
	
	std::ostream& Object::writeToStream(std::ostream& os) const{
		return os << name;
	}
	
	std::ostream& operator<<(std::ostream& os, const XML::Object& obj){
		return obj.writeToStream(os);
	}
	
	Object* Object::copy() const{
		return new Object(*this);
	}

//-----------------------------------STRING-----------------------------------------------//
	
	String::String():XML::Object(){}
	String::String(const std::string& Name, Tag * Parent):XML::Object(Name,Parent){
		type = XML_STRING;
	}
	String::String(const String& other):Object(other){}
	
	String* String::copy() const{
		return new String(*this);
	}
	
	String& String::operator+=(const String& rhs){
		name += rhs.name;
		return *this;
	}
	
	String operator+(String lhs, const String& rhs){
		lhs += rhs;
		return lhs;
	}
	
	std::ostream& String::writeToStream(std::ostream& os) const{
		return os << XML::EscapeString(name);
	}
	
//-----------------------------------TAG-----------------------------------------------//
	
	Tag::~Tag(){
		if(deleteChildTagsOnDestruction){
			clearChildren();
		}
	}
	
	Tag::Tag():XML::Object(), deleteChildTagsOnDestruction(true){
		type = XML_TAG;
	}
	
	Tag::Tag(const std::string& Name, Tag* Parent):XML::Object(Name,Parent), deleteChildTagsOnDestruction(true){
		type = XML_TAG;
	}
	
	Tag::Tag(const Tag& other):XML::Object(other), deleteChildTagsOnDestruction(true){
		this->attributes = other.attributes;
		for(unsigned int i = 0; i < other.children.size(); ++i){
			this->children.push_back(XML::Copy(other.children[i]));
		}
	}
	
	Tag* Tag::copy() const{
		return new Tag(*this);
	}
	
	
	XML::Object* Tag::addChild(XML::Object* obj){
		obj->parent = this;
        //combine multiple strings together
		if(obj->getType() == XML_STRING && children.size() > 0 && children.back()->getType() == XML_STRING){
			children.back()->name += obj->name;
			delete obj;
			obj = children.back();
		}else{
			children.push_back(obj);
		}
		return obj;                
	}
	
	String* Tag::addChildString(const std::string& value){
		return (String*)addChild(new String(value, this));
	}
	
	Tag* Tag::addChildTag(const std::string& childName){
		return (Tag*)addChild(new Tag(childName, this));
	}
	
	std::string& Tag::firstText(){
		if(children.size() == 0){
			return addChildString("")->name;
		}else{
			if(children[0]->getType() == XML_STRING){
				return children[0]->name;
			}else{
				String* newStr = new String("",this);
				children.insert(children.begin(),newStr);
				return newStr->name;
			}
		}
	}
	
	const std::string& Tag::firstText() const{
		return children[0]->name;
	}
	
	void Tag::clearChildren(){
		if(deleteChildTagsOnDestruction){
			unsigned int i = children.size();
			while(i-->0){
				if(children[i] != NULL){
					delete children[i];
					children[i] = NULL;
				}
			}
		}
		children.clear();
	}
	
	void Tag::clear(){
		name.clear();
		clearChildren();
		attributes.clear();
	}
	
	Tag* Tag::childWithName(const std::string& childName){
		return const_cast<Tag*>(static_cast<const Tag*>(this)->childWithName(childName));
	}
	
	const Tag* Tag::childWithName(const std::string& childName) const{
		for(unsigned int i = 0; i < children.size(); ++i){
			if(children[i]->getType() == XML_TAG && children[i]->name == childName){
				return (Tag*)children[i];
			}
		}
		return NULL;
	}
	
	Tag* Tag::lastChildWithName(const std::string& childName){
		unsigned int sz = children.size();
		while(sz-- > 0){
			if(children[sz]->getType() == XML_TAG && children[sz]->name == childName){
				return (Tag*)children[sz];
			}
		}
		return NULL;
	}
		
	const Tag* Tag::lastChildWithName(const std::string& childName) const{
		return lastChildWithName(childName);
	}
	
	std::vector<Tag*> Tag::childrenWithName(const std::string& childName){
		std::vector<Tag*> output;
		for(unsigned int i = 0; i < children.size(); ++i){
			if(children[i]->getType() == XML_TAG && children[i]->name == childName){
				output.push_back((Tag*)children[i]);
			}
		}
		return output;
	}
	
	const std::vector<Tag*> Tag::childrenWithName(const std::string& childName) const{
		return childrenWithName(childName);
	}
	
	std::vector<XML::String*> Tag::stringChildren(){
		std::vector<XML::String*> output;
		for(unsigned int i = 0; i < children.size(); ++i){
			if(children[i]->getType() == XML_STRING){
				output.push_back((XML::String*)children[i]);
			}
		}
		return output;
	}
	const std::vector<String*> Tag::stringChildren() const{
		return stringChildren();
	}
	
	void Tag::addChildrenFromTag(XML::Tag* other){
		for(unsigned int i = 0; i < other->children.size(); ++i){
			addChild(XML::Copy(other->children[i]));
		}
	}
	
	const std::map<std::string, Tag*> Tag::childrenToMap() const{
		std::map<std::string, Tag*> output;
		unsigned int i = children.size();
		while(i-- > 0){
			if(children[i]->getType() == XML_TAG){
				output[children[i]->name] = (XML::Tag*)children[i];
			}
		}
		return output;
	}
	
	std::ostream& Tag::writeAttributes(std::ostream& os) const{
		for(std::map<std::string, std::string>::const_iterator it = attributes.begin(); it != attributes.end(); ++it){
			if(it->first != "/"){
				os << ' ' << it->first;
				if(!it->second.empty()){
					os << "=\"" << it->second << '"';
				}
			}
		}
		return os;
	}
	
	std::ostream& Tag::writeChildren(std::ostream& os) const{
		for(unsigned int i = 0; i < children.size(); ++i){
			switch(children[i]->getType()){
				case XML_TAG:
					os << *((Tag*)children[i]);
					break;
				case XML_STRING:
					os << *((String*)children[i]);
					break;
				default:
					os << *children[i];
			}
		}
		return os;
	}
	
	std::ostream& Tag::writeToStream(std::ostream& os) const{
		//name
		os << '<' << name;
		//attributes
		writeAttributes(os);
		
		if(children.size() == 0){ //close self
			os << " />";
		}else{
			os << '>';
			//children
			writeChildren(os);
			os << "</" << name << '>';
		}
		
		return os;
	}
	
	//Read the start tag
	//ie. "<name attr1="val1" ...>"
	//or "name attr1="val1" ...>"
	Tag* ReadStartTag(Tag* output, std::istream& is, char startingChar = '\0'){
		if(output == NULL){
			output = new Tag();
		}else{
			output->clear();
		}
		
		char c = '\0';
		//Remove Leading Spaces
		if(startingChar != '\0'){
			output->name.push_back(startingChar);
		}else{
			while(is.get(c) && c != '<'){}
		}

		bool tag_done = false;
		//get name
		while(is.get(c)){
			if(c == ' '){
				break;
			}else if(c == '>'){
				break;
			}else if(c == '/'){
				is.get(c);
				if(c == '>'){
					tag_done = true;
					break;
				}else if(c == ' '){
					break;
				}else{
					output->name.push_back('/');
					output->name.push_back(c);
				}
			}else{
				output->name.push_back(c);
			}
		}
		
		//check for attributes
		if(!output->name.empty() && !tag_done){
			do{
				if(c == '>'){
					break;
				}else if(c == ' '){
					
				}else{ //extract attribute
					if(is.peek() == '>'){
						if(c=='?'){
							break;
						}else if(c=='/'){
							tag_done = true;
							is.get(c);
							break;
						}
					}
					
					std::string attributeName;
					std::string attributeValue;
					attributeName.push_back(c);
					while(is.get(c)){
						if(c == '='){
							if(is.get(c) && c == '"'){
								while(is.get(c) && c != '"'){
									attributeValue.push_back(c);
								}
								break;
							}else{
								break;
							}
						}else if(c ==' '){
							if(!attributeName.empty()){
								break;
							}
						}else if(c=='>'){
							break;
						}else{
							attributeName.push_back(c);
						}
					}
					if(!attributeName.empty()){
						output->attributes[attributeName] = attributeValue;
					}
				}
			}while(c != '>' && is.get(c));
		}
		
		if(tag_done){
			output->attributes["/"] = "";
		}
		
		return output;
	}
	
	Tag* ReadStartTag(std::istream& is, char startingChar = '\0'){
		return ReadStartTag(NULL, is, startingChar);
	}
	
	//assumes the next data in the stream is in the form -> "tagName>....."
	bool ReadEndTag(std::istream& is, const std::string& tagName, std::string& red){
		char c;
		unsigned int i = 0;
		red.clear();
		while(is.get(c)){
			red.push_back(c);
			if(i == tagName.length()){
				if(c=='>')	break;
				else return false;
			}else if(c != tagName[i++]){
				return false;
			}
		}
		return true;
	}
	
	//Read the internal content of the tag, starting after the begin tag <name>... and requires that the provided tag has a name set.
	void ReadInnerTag(std::istream& is, Tag* tag){
		if(!tag->name.empty()){
			std::string tmp;
			char c;
			while(std::getline(is,tmp,'<')){
				tag->addChildString(DeEscapeString(tmp));
				
				//check if this is the end tag
				if(is.get(c)){
					if(c == '/'){
						if(ReadEndTag(is,tag->name, tmp)){
							break;
						}else{
							tag->addChildString(DeEscapeString(tmp));
						}
					}else{ //this is the start of an inner tag.
						Tag * newTag = Tag::FromStream(is,c);
						if(newTag != NULL){
							tag->addChild(newTag);
						}
					}
				}
			}
		}
	}
	
	Tag* Tag::FromStream(Tag* output, std::istream& is,char startingChar, bool returnNull){
		output = ReadStartTag(output, is, startingChar);
		if(output->name.empty()){
			if(returnNull){
				delete output;
				output = NULL;
			}
		}else{
			std::map<std::string, std::string>::iterator fnd = output->attributes.find("/");
			if(fnd != output->attributes.end() && fnd->second.empty()){ //self closing tag
				
			}else{
				ReadInnerTag(is,output);
			}
		}
		return output;
	}
	
	Tag* Tag::FromStream(std::istream& is,char startingChar, bool returnNull){
		return Tag::FromStream(NULL,is,startingChar, returnNull);
	}
	
	std::istream& operator>>(std::istream& is, Tag& output){
		Tag::FromStream(&output,is);
		return is;
	}
	
//-----------------------------------DOCUMENT----------------------------------------------//
	
	Document::Document():declaration(NULL), root(NULL){}
	Document::Document(const Document& doc):declaration(doc.declaration), root(doc.root){}
	
	void Document::deleteTags(){
		if(declaration != NULL){
			delete declaration;
			declaration = NULL;
		}
		if(root != NULL){
			delete root;
			root = NULL;
		}
	}
	
	std::map<std::string,std::string>& Document::Declaration(){
		if(declaration == NULL){
			createDeclaration();
		}
		return declaration->attributes;
	}
	
	Tag* Document::createDeclaration(){
		if(declaration == NULL){
			declaration = new Tag("?xml");
			declaration->attributes["version"] = "1.0";
		}
		return declaration;
	}
	
	
	Tag* Document::createRoot(const std::string& rootName){
		if(root == NULL){
			root = new Tag(rootName);
		}
		return root;
	}
	
	Tag* Document::createRoot(){
		return createRoot("_root");
	}
	
	std::ostream& Document::writeToStream(std::ostream& os) const{
		if(declaration != NULL){
			os << '<' << declaration->name;
			//attributes
			declaration->writeAttributes(os) << "?>";
		}
		if(root->name == "_root"){
			root->writeChildren(os);
		}else{
			root->writeToStream(os);
		}
		return os;
	}

	Document Document::FromStream(std::istream& is, const std::string& rootName){
		Document output;
		
		//create root
		output.root = new Tag(rootName);
		
		Tag * newTag = ReadStartTag(is);
		if(newTag->name == "?xml"){
			output.declaration = newTag;
		}else if(!newTag->name.empty()){
			output.createDeclaration();
			ReadInnerTag(is,newTag);
			output.root->addChild(newTag);
		}else{
			delete newTag;
		}
		
		while(newTag != NULL && !newTag->name.empty()){
			newTag = Tag::FromStream(is);
			if(newTag != NULL){
				output.root->addChild(newTag);
			}
		}
		
		return output;
	}
	
	Document Document::FromStream(std::istream& is){
		return Document::FromStream(is,"_root");
	}
	
	std::ostream& operator<<(std::ostream& os, const XML::Document& doc){
		return doc.writeToStream(os);
	}
	
	std::istream& operator>>(std::istream& is, XML::Document& output){
		output = XML::Document::FromStream(is);
		return is;
	}
};