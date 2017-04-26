#ifndef _XML_H
#define _XML_H

#include <string>
#include <iostream>
#include <vector>
#include <map>

namespace XML{
	
	//Escape Info
	extern const unsigned int NumberOfEscapeCharacters;
	extern const std::string escapeCharacters;
	extern const std::string escapeStrings[];
	
	std::string EscapeString(const std::string& str);
	std::string DeEscapeString(const std::string& str);
	
	//The different types of XML objects
	enum XML_OBJECT_TYPE{
		XML_OBJECT,
		XML_STRING,
		XML_TAG
	};
	
	struct Tag;
	
	//Base object
	struct Object{
	protected:
		XML_OBJECT_TYPE type;
		
	public:
	
		Tag * parent;
		std::string name;
		
		Object(Tag * Parent = NULL);
		Object(const std::string& Name, Tag * Parent = NULL);
		Object(const Object& other);
		
		XML_OBJECT_TYPE getType() const{	return type;	}
		
		virtual std::ostream& writeToStream(std::ostream& os) const;
		
		friend std::ostream& operator<<(std::ostream& os, const XML::Object& obj);
		
		virtual Object* copy() const;
	};
	
	//used to copy any XML type object
	Object* Copy(const Object* other);
	
	//XML String 
	struct String : public XML::Object{
		String(Tag * Parent = NULL);
		String(const std::string& Name, Tag * Parent = NULL);
		String(const String& other);
		
		virtual String* copy() const;
		
		String& operator+=(const String& rhs);
		friend String operator+(String lhs, const String& rhs);
		
		virtual std::ostream& writeToStream(std::ostream& os) const;
	};
	
	//XML Tag
	class Tag : public XML::Object{
	public:
		typedef std::vector<XML::Object*> vec_type;
		
		//std::string content;
		std::map<std::string, std::string> attributes;
		//std::vector<Tag> children;
		vec_type children;
		
		~Tag();
		
		Tag(Tag* Parent = NULL);
		Tag(const std::string& Name, Tag* Parent = NULL);
		Tag(const Tag& other);
		
		virtual Tag* copy() const;
		
		//setting this bool determines if the children of this tag should be detroyed, true by default
		bool deleteChildTagsOnDestruction;
		
		//Add children to the tag
		XML::Object* addChild(XML::Object* obj);
		String* addChildString(const std::string& value);
		Tag* addChildTag(const std::string& childName);
		
		//If the first child is a string, return it, otherwise push a string to the front of the child list and return that.
		std::string& firstText();
		
		//destroy the children
		void clearChildren();
		
		//clear the name, children, and attributes
		void clear();
		
		//get the first child with name
		Tag* childWithName(const std::string& childName);
		const Tag* childWithName(const std::string& childName) const;
		
		//get the first child with name, starting backward
		Tag* lastChildWithName(const std::string& childName);
		const Tag* lastChildWithName(const std::string& childName) const;
		
		//Get all children with the name
		std::vector<Tag*> childrenWithName(const std::string& childName);
		const std::vector<Tag*> childrenWithName(const std::string& childName) const;
		
		//return the children which are strings
		std::vector<String*> stringChildren();
		const std::vector<String*> stringChildren() const;
		
		//Return a map of the child tags with the key being their name.
		//Only returns the first instance of children with the same name.
		const std::map<std::string, Tag*> childrenToMap() const;
		
		//Write the attributes to a stream
		std::ostream& writeAttributes(std::ostream& os) const;
		//Write the children to a stream
		std::ostream& writeChildren(std::ostream& os) const;
		
		virtual std::ostream& writeToStream(std::ostream& os) const;
		
		//Retreive a tag from a stream.
		
		//If output != NULL a new tag is created, if not, output is returned.
		//If startingChar != '\0' startingChar is appended to the front of the tag name. (This is useful for other parsing).
		//If returnNull == true the return/output is set to NULL if no tag was found.
		static Tag* FromStream(Tag* output, std::istream& is, char startingChar = '\0', bool returnNull = true);
		static Tag* FromStream(std::istream& is, char startingChar = '\0', bool returnNull = true);
		
		friend std::istream& operator>>(std::istream& is, Tag& output);
	};
	
	//Struct to store XML Document information.
	struct Document{
		//The declaration information
		Tag* declaration;
		
		//The root of the document
		Tag* root;

		Document();
		Document(const Document& doc);
		
		//access the attributes of the declaration tag.
		std::map<std::string,std::string>& Declaration();
		
		//Destroy the root and declaration tags if not NULL.
		//This is not done on the document destruction.
		void deleteTags();
		
		//Write the document to a stream
		std::ostream& writeToStream(std::ostream& os) const;
		
		//Create a new tag as the declaration tag.
		Tag* createDeclaration();
		
		//create a new root tag, with the provided name.
		//If no name is provided then "_root" is used.
		//If the name of the root tag is "_root" then it will not be written to a stream.
		Tag* createRoot();
		Tag* createRoot(const std::string& rootName);
		
		//Read a document from a stream.
		static Document FromStream(std::istream& os, const std::string& rootName);
		static Document FromStream(std::istream& os);
		
		friend std::istream& operator>>(std::istream& is, Document& output);
		friend std::ostream& operator<<(std::ostream& os, const Document& doc);
	};
	
}; //end namespace XML

#endif //_XML_H
