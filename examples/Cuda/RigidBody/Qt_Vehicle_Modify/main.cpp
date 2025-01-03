#include <QtApp.h>

#include <SceneGraph.h>

#include <RigidBody/Vechicle.h>

#include <GLRenderEngine.h>
#include <GLPointVisualModule.h>
#include <GLSurfaceVisualModule.h>
#include <GLWireframeVisualModule.h>

#include <Mapping/DiscreteElementsToTriangleSet.h>
#include <Mapping/ContactsToEdgeSet.h>
#include <Mapping/ContactsToPointSet.h>
#include <Mapping/AnchorPointToPointSet.h>

#include "Collision/NeighborElementQuery.h"
#include "Collision/CollistionDetectionTriangleSet.h"
#include "Collision/CollistionDetectionBoundingBox.h"

#include <Module/GLPhotorealisticInstanceRender.h>

#include <BasicShapes/PlaneModel.h>

#include "GltfLoader.h"


using namespace std;
using namespace dyno;

std::shared_ptr<SceneGraph> creatCar()
{
	std::shared_ptr<SceneGraph> scn = std::make_shared<SceneGraph>();

	auto jeep = scn->addNode(std::make_shared<ArticulatedBody<DataType3f>>());

	auto prRender = std::make_shared<GLPhotorealisticInstanceRender>();
	jeep->inTextureMesh()->connect(prRender->inTextureMesh());
	jeep->stateInstanceTransform()->connect(prRender->inTransform());
	jeep->graphicsPipeline()->pushModule(prRender);

	uint N = 1;

	for (uint i = 0; i < N; i++)
	{
		Vec3f tr = i * Vec3f(3.0f, 0.0f, 0.0f);

		BoxInfo box1, box2, box3, box4, box5, box6;
		//car body
		box1.center = Vec3f(0, 1.171, -0.011) + tr;
		box1.halfLength = Vec3f(1.011, 0.793, 2.4);


		box2.center = Vec3f(0, 1.044, -2.254) + tr;
		box2.halfLength = Vec3f(0.447, 0.447, 0.15);

		
		/*CapsuleInfo capsule1, capsule2, capsule3, capsule4;

		capsule1.center = Vec3f(0.812, 0.450, 1.722) + tr;
		capsule1.rot = Quat1f(M_PI / 2, Vec3f(0, 0, 1));
		capsule1.halfLength = 0.1495;
		capsule1.radius = 0.450;
		capsule2.center = Vec3f(-0.812, 0.450, 1.722) + tr;
		capsule2.rot = Quat1f(M_PI / 2, Vec3f(0, 0, 1));
		capsule2.halfLength = 0.1495;
		capsule2.radius = 0.450;
		capsule3.center = Vec3f(-0.812, 0.450, -1.426) + tr;
		capsule3.rot = Quat1f(M_PI / 2, Vec3f(0, 0, 1));
		capsule3.halfLength = 0.1495;
		capsule3.radius = 0.450;
		capsule4.center = Vec3f(0.812, 0.450, -1.426) + tr;
		capsule4.rot = Quat1f(M_PI / 2, Vec3f(0, 0, 1));
		capsule4.halfLength = 0.1495;
		capsule4.radius = 0.450;*/
		SphereInfo sphere1, sphere2, sphere3, sphere4;
		sphere1.center = Vec3f(0.812, 0.450, 1.722) + tr;
		sphere1.radius = 0.450;
		sphere2.center = Vec3f(-0.812, 0.450, 1.722) + tr;
		sphere2.radius = 0.450;
		sphere3.center = Vec3f(-0.812, 0.450, -1.426) + tr;
		sphere3.radius = 0.450;
		sphere4.center = Vec3f(0.812, 0.450, -1.426) + tr;
		sphere4.radius = 0.450;

		box3.center = Vec3f(0.812, 0.450, 1.722) + tr;
		box3.halfLength = Vec3f(0.15, 0.15, 0.15) + tr;
		box4.center = Vec3f(-0.8122, 0.450, 1.722) + tr;
		box4.halfLength = Vec3f(0.15, 0.15, 0.15) + tr;

		

		


		RigidBodyInfo rigidbody;

		rigidbody.bodyId = i;

		Vec3f offset = Vec3f(0.0f, -0.721, 0.159);
		rigidbody.offset = offset;
		auto bodyActor = jeep->addBox(box1, rigidbody, 100);

		rigidbody.offset = Vec3f(0.0f);

		auto spareTireActor = jeep->addBox(box2, rigidbody);

		Real wheel_velocity = 10;

		/*auto frontLeftTireActor = jeep->addCapsule(capsule1, rigidbody, 100);
		auto frontRightTireActor = jeep->addCapsule(capsule2, rigidbody, 100);
		auto rearLeftTireActor = jeep->addCapsule(capsule3, rigidbody, 100);
		auto rearRightTireActor = jeep->addCapsule(capsule4, rigidbody, 100);*/
		auto frontLeftTireActor = jeep->addSphere(sphere1, rigidbody, 500);
		auto frontRightTireActor = jeep->addSphere(sphere2, rigidbody, 500);
		auto rearLeftTireActor = jeep->addSphere(sphere3, rigidbody, 500);
		auto rearRightTireActor = jeep->addSphere(sphere4, rigidbody, 500);

		auto frontLeftActor = jeep->addBox(box3, rigidbody, 5000);
		auto frontRightActor = jeep->addBox(box4, rigidbody, 5000);

		//auto rearActor = jeep->addBox(box6, rigidbody, 25000);

		//front rear
		auto& joint1 = jeep->createHingeJoint(frontLeftTireActor, frontLeftActor);
		joint1.setAnchorPoint(frontLeftTireActor->center);
		//joint1.setMoter(wheel_velocity);
		joint1.setAxis(Vec3f(1, 0, 0));

		auto& joint2 = jeep->createHingeJoint(frontRightTireActor, frontRightActor);
		joint2.setAnchorPoint(frontRightTireActor->center);
		//joint2.setMoter(wheel_velocity);
		joint2.setAxis(Vec3f(1, 0, 0));

		//back rear
		auto& joint3 = jeep->createHingeJoint(rearLeftTireActor, bodyActor);
		joint3.setAnchorPoint(rearLeftTireActor->center);
		//joint3.setMoter(wheel_velocity);
		joint3.setAxis(Vec3f(1, 0, 0));

		auto& joint4 = jeep->createHingeJoint(rearRightTireActor, bodyActor);
		joint4.setAnchorPoint(rearRightTireActor->center);
		//joint4.setMoter(wheel_velocity);
		joint4.setAxis(Vec3f(1, 0, 0));


		auto& joint5 = jeep->createFixedJoint(bodyActor, spareTireActor);
		joint5.setAnchorPoint((bodyActor->center + spareTireActor->center) / 2);


		auto& joint6 = jeep->createHingeJoint(bodyActor, frontLeftActor);
		joint6.setAnchorPoint(frontLeftActor->center);
		joint6.setAxis(Vec3f(0, 1, 0));
		joint6.setRange(0, 0);

		auto& joint7 = jeep->createHingeJoint(bodyActor, frontRightActor);
		joint7.setAnchorPoint(frontRightActor->center);
		joint7.setAxis(Vec3f(0, 1, 0));
		joint7.setRange(0, 0);

		


		jeep->bind(bodyActor, Pair<uint, uint>(5, i));
		jeep->bind(spareTireActor, Pair<uint, uint>(4, i));
		jeep->bind(frontLeftTireActor, Pair<uint, uint>(0, i));
		jeep->bind(frontRightTireActor, Pair<uint, uint>(1, i));
		jeep->bind(rearLeftTireActor, Pair<uint, uint>(2, i));
		jeep->bind(rearRightTireActor, Pair<uint, uint>(3, i));
	}

	auto gltf = scn->addNode(std::make_shared<GltfLoader<DataType3f>>());
	gltf->setVisible(false);
	gltf->varFileName()->setValue(getAssetPath() + "Jeep/JeepGltf/jeep.gltf");

	gltf->stateTextureMesh()->connect(jeep->inTextureMesh());

	auto plane = scn->addNode(std::make_shared<PlaneModel<DataType3f>>());
	plane->varLocation()->setValue(Vec3f(0, 0, 0));
	plane->varScale()->setValue(Vec3f(300.0f));
	plane->stateTriangleSet()->connect(jeep->inTriangleSet());



	//Visualize contact points
	auto cdBV = std::make_shared<CollistionDetectionTriangleSet<DataType3f>>();
	jeep->stateTopology()->connect(cdBV->inDiscreteElements());
	jeep->inTriangleSet()->connect(cdBV->inTriangleSet());
	jeep->graphicsPipeline()->pushModule(cdBV);

	auto contactPointMapper = std::make_shared<ContactsToPointSet<DataType3f>>();
	cdBV->outContacts()->connect(contactPointMapper->inContacts());
	jeep->graphicsPipeline()->pushModule(contactPointMapper);

	auto contactsRender = std::make_shared<GLPointVisualModule>();
	contactsRender->setColor(Color(1, 0, 0));
	contactsRender->varPointSize()->setValue(0.1f);
	contactPointMapper->outPointSet()->connect(contactsRender->inPointSet());
	jeep->graphicsPipeline()->pushModule(contactsRender);

	auto contactMapper = std::make_shared<ContactsToEdgeSet<DataType3f>>();
	cdBV->outContacts()->connect(contactMapper->inContacts());
	contactMapper->varScale()->setValue(2);
	jeep->graphicsPipeline()->pushModule(contactMapper);

	auto wireRender = std::make_shared<GLWireframeVisualModule>();
	wireRender->setColor(Color(1, 0, 0));
	contactMapper->outEdgeSet()->connect(wireRender->inEdgeSet());
	jeep->graphicsPipeline()->pushModule(wireRender);


	//Visualize rigid bodies

	/*auto mapper = std::make_shared<DiscreteElementsToTriangleSet<DataType3f>>();
	jeep->stateTopology()->connect(mapper->inDiscreteElements());
	jeep->graphicsPipeline()->pushModule(mapper);

	auto sRender = std::make_shared<GLSurfaceVisualModule>();
	sRender->setColor(Color(0.3f, 0.5f, 0.9f));
	sRender->setAlpha(0.8f);
	sRender->setRoughness(0.7f);
	sRender->setMetallic(3.0f);
	mapper->outTriangleSet()->connect(sRender->inTriangleSet());
	jeep->graphicsPipeline()->pushModule(sRender);*/

	return scn;
}

int main()
{
	QtApp app;
	app.setSceneGraph(creatCar());
	app.initialize(1280, 768);

	//Set the distance unit for the camera, the fault unit is meter
	app.renderWindow()->getCamera()->setUnitScale(3.0f);

	app.mainLoop();

	return 0;
}


